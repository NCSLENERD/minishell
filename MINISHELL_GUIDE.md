# Minishell — Guide de travail (duo)

> Base de départ : **ShellPy** (shell Python existant) · Sujet **v10.0** · Répartition **parsing / exécution**
> Départ **J1 = jeu 30 juil 2026** · Code gelé **J30 = ven 28 août** · Blackhole **≈ J40 = lun 7 sept**
>
> Ce document contient le plan (Partie I), les notions de fond nécessaires pour l'exécuter (Partie II),
> et ce qui reste à décider (Partie III). Il ne contient volontairement **aucun code**.

---

## Sommaire

**Partie I — Le plan**
1. [Calibrage](#1-calibrage)
2. [Décisions gelées](#2-décisions-gelées)
3. [Héritage ShellPy](#3-héritage-shellpy)
4. [Les cinq phases](#4-les-cinq-phases)
5. [Répartition et territoires partagés](#5-répartition-et-territoires-partagés)
6. [Signaux d'alarme](#6-signaux-dalarme)
7. [Le contrat des quatre structs](#7-le-contrat-des-quatre-structs--livrable-de-phase-0)
8. [Checklist avant de geler le code](#8-checklist-avant-de-geler-le-code)

**Partie II — Le cours**
9. [Lexer et parser : la frontière](#9-lexer-et-parser--la-frontière)
10. [Le pré-traitement, une impasse](#10-le-pré-traitement-une-impasse)
11. [Les quotes : le problème central](#11-les-quotes--le-problème-central)
12. [Structures de données en C](#12-structures-de-données-en-c--tableau-liste-arbre)
13. [Liste plate ou arbre](#13-la-structure-de-la-ligne--liste-plate-ou-arbre)
14. [`argv` : un seul tableau](#14-argv--un-seul-tableau)
15. [Les redirections : une liste ordonnée](#15-les-redirections--une-liste-ordonnée)
16. [`&&` et `||`](#16--et-)
17. [fork, execve, et pourquoi pas de threads](#17-fork-execve-et-pourquoi-pas-de-threads)
18. [Les codes de sortie](#18-les-codes-de-sortie)

**Partie III — [En attente](#partie-iii--en-attente)**

---
---

# Partie I — Le plan

## 1. Calibrage

40 jours de calendrier ne sont pas 40 jours de code. Le blackhole se referme quand le projet est
**validé**, pas quand il est fini : il faut caler 3 soutenances avec des pairs disponibles, puis laisser
passer Deepthought. Compter 7 à 10 jours.

| | |
|---|---|
| **Deadline réel du code** | **J30 (ven 28 août)** — pas J40 |
| **Budget disponible** | 30 jours × 3,5 h × 2 personnes ≈ **210 h de duo** |
| **Coût réel d'un mandatory propre** | ≈ 200–220 h en duo |
| **Verdict** | **Faisable, marge nulle.** |

### Les trois conséquences à accepter

- **Aucun gros refactor n'est finançable après J16.** Le design des quotes doit être juste du premier
  coup — c'est toute la raison d'être de la Phase 0.
- **Le bonus n'est décidé qu'au Gate J23**, jamais avant. Un mandatory imparfait annule intégralement le
  bonus (le sujet est explicite sur ce point).
- **Une semaine perdue se paie sur la Phase 4** (chasse aux fuites et aux crashs), c'est-à-dire au pire
  endroit possible : un minishell complet qui fuit ou segfault prend 0.

---

## 2. Décisions gelées

Signées à deux, non rediscutables. Chaque ligne est expliquée en détail en Partie II.

| Décision | Choix | Pourquoi | Détail |
|---|---|---|---|
| **Suivi des quotes** | Le token est une **struct**, pas une chaîne : une suite de morceaux, chacun portant son type de quote (aucune / simple / double) | Le lexer est le seul endroit qui voit jamais `'` et `"`. Ce qu'il ne note pas est perdu pour toujours | [§11](#11-les-quotes--le-problème-central) |
| **Forme du parser** | **Arbre**, avec un champ discriminant (commande / pipeline / futur `&&` `\|\|`) | Quelques heures maintenant, plusieurs jours si conversion tardive | [§13](#13-la-structure-de-la-ligne--liste-plate-ou-arbre) |
| **Arguments** | **Un seul tableau**, case 0 = la commande, terminé par `NULL` | `execve` l'exige. Supprime le bug `grep grep f` | [§14](#14-argv--un-seul-tableau) |
| **Redirections** | **Liste ordonnée**, chaque élément portant son type (`<` `>` `>>` `<<`) et sa cible | Bash les applique toutes, de gauche à droite | [§15](#15-les-redirections--une-liste-ordonnée) |
| **Environnement** | **Une seule table**, propriétaire unique, passée en paramètre | La règle « une seule globale, contenant uniquement le numéro de signal » interdit de la mettre en global | [§7](#7-le-contrat-des-quatre-structs--livrable-de-phase-0) |
| **Reconnaissance des opérateurs** | **Dans l'automate du lexer**, caractère par caractère. Aucun pré-traitement de la ligne | Un remplacement global ne sait pas s'il est dans une quote : il corrompt les données | [§10](#10-le-pré-traitement-une-impasse) |
| **Threads** | **Aucun.** Interdits par le sujet, et inutilisables de toute façon | `execve` remplace le processus entier | [§17](#17-fork-execve-et-pourquoi-pas-de-threads) |

### Structure de nœud retenue (hybride)

| Genre de boîte | Enfants | Ce que c'est |
|---|---|---|
| **COMMANDE** | aucun (feuille) | `argv` + sa liste de redirections |
| **PIPELINE** | une **liste** de COMMANDE | Contient 1 commande s'il n'y a pas de `\|` |
| **ET** / **OU** | deux enfants, gauche et droite | Binaire. Chaque enfant est un PIPELINE ou un autre ET/OU |

Pour le mandatory, la racine est **toujours** un PIPELINE et les boîtes ET/OU n'existent jamais. Le parser
réserve leur place sans les produire. Justification en [§16](#16--et-).

---

## 3. Héritage ShellPy

> **Constat de calibrage : le parsing n'est pas à 60 %, il est à ~15 %.**
> La ligne `shlex.split(line)` (`tokenizer.py:29`) fait à elle seule les trois morceaux les plus durs du
> projet — reconnaissance des quotes, découpage en mots, suppression des quotes. `shlex` n'existe pas en
> C : 36 lignes de Python cachent 350 à 450 lignes de C normées.
> Bonne nouvelle : les ~200 lignes hors-sujet à supprimer compensent, la roadmap tient.

### On garde

- Le découpage en modules `parsing / execution / cmd_built_in / utils` — aide aussi pour la Norme
- `shell_state` : un état central passé partout — architecture **obligatoire** ici
- `handle_redir` isolé dans son propre fichier
- La séparation `parse_simple` / `parse_pipeline` — c'est déjà une descente récursive
- Le pipeline qui contient une liste de commandes — c'est le bon choix, voir [§16](#16--et-)
- La silhouette du dict de commande, et son champ `type` (c'est un discriminant)
- L'ordre lexer → parser → expansion → exécution

### On refait

- **Le lexer entièrement** : automate à états, zéro pré-traitement de la ligne
- **La place de l'expansion** : elle doit venir *après* le découpage, avec l'info de quote intacte. La
  suppression des quotes est la **dernière** étape
- Le tag `__NO_EXPAND__` (`tokenizer.py:9-14`) → remplacé par un champ de struct
- Reconstruction de `argv` par comparaison de chaînes (`parser.py:48`) → parcours unique gauche→droite
- Slots `stdin`/`stdout` uniques (`parser.py:8-9`) → liste ordonnée
- Messages d'erreur français (`cd.py:12`, `executor_simple.py:43`) → format bash, sur **stderr**
- `shell_state["?"] = -1` (`executor_simple.py:24`) → ce n'est pas un code de sortie valide

### On supprime

| Quoi | Pourquoi |
|---|---|
| `func_if.py` (123 l.) | hors sujet |
| `func_for.py` (54 l.) | hors sujet |
| `;` (`tokenizer.py:27`) | le sujet **interdit** de l'interpréter |
| `&`, `background.py`, la table de processus en fond | hors sujet |
| `import threading` (`main.py:9`) | import mort, et threads interdits |
| `help` | hors sujet |
| `user_vars` + `is_affect` (`users_vars.py`) | une seule table d'env suffit |

### Absents du Python, à écrire de zéro

`<<` heredoc · `export` · `env` · `pwd` · `echo -n` · la gestion des signaux · les codes de sortie conformes

### Bugs mesurés sur la base Python

Tests réellement exécutés sur ShellPy, avec `A="a b"` :

| entrée | résultat ShellPy | bash | cause |
|---|---|---|---|
| `echo "a\|b"` | `['echo', 'a \| b']` | `a\|b` | pré-traitement aveugle |
| `echo ">"` | `['echo', ' > ']` | `>` | pré-traitement aveugle |
| `echo "$A"` | `['echo', 'a b']` | 1 arg : `a b` | quotes déjà jetées |
| `echo $A` | `['echo', 'a b']` | 2 args : `a`, `b` | **identique au cas précédent** |
| `echo abc$A` | `['echo', 'abc$A']` | 2 args : `abca`, `b` | test `startswith("$")` |
| `grep grep f` | `cmd=grep, args=['f']` | 3 mots | comparaison de chaînes |
| `echo hi > a > b` | seul `b` | `a` **et** `b` créés | case écrasée |
| `cat < f1 < f2` | seul `f2` | les deux ouverts | case écrasée |
| `echo a << EOF` | `stdin={'file':'<'}` | heredoc | pas de type `<<` |

---

## 4. Les cinq phases

### Phase 0 — Le contrat · J1–J2 · 30–31 juil

**À deux, sur le même écran, aucun code séparé.** La phase que tout le monde saute et qui coûte une
semaine plus tard.

- Écrire le contrat des quatre structs ([§7](#7-le-contrat-des-quatre-structs--livrable-de-phase-0)) :
  champs, propriétaire, libération, durée de vie
- Définir le mécanisme de suivi des quotes dans le détail — le seul point où la base Python ne donne rien
- Trancher qui possède la table d'environnement (`export`, `unset`, l'expansion y touchent tous les trois)
- Découper explicitement le heredoc : qui le lit, qui branche le descripteur
- Repo git, stratégie de branches, **un seul propriétaire du Makefile**, norminette dès le premier fichier
- Écrire à la main ~25 cas de test tirés de bash réel : entrée → sortie attendue → `$?` attendu.
  C'est la spec exécutable, et elle servira en soutenance

> **Gate J2** — Vous savez tous les deux dessiner les quatre structs au tableau, à l'identique, sans
> regarder. Le fichier de tests existe.

### Phase 1 — Deux squelettes en parallèle · J3–J9 · 1–7 août

| Toi — parsing | Binôme — exécution |
|---|---|
| Boucle readline + historique | fork / execve / résolution du PATH |
| L'automate à états du lexer : gestion des quotes, info de quote **conservée** par morceau | Sur une structure de commande **écrite en dur dans le code** |
| Production de tokens affichables pour vérification | Une seule commande, aucun pipe |

> **⚠ Le point critique de toute la roadmap.**
> Il ne doit **surtout pas** attendre ton parser. Il fabrique une fausse commande à la main et travaille
> dessus. Deux moitiés qui se compilent séparément contre une interface figée avancent en parallèle ; deux
> moitiés qui se compilent ensemble avancent en série. C'est ce mock qui fait gagner une semaine.

> **Gate J9** — `ls -la` s'exécute de bout en bout depuis une struct codée en dur, et le lexer sort les
> bons tokens sur 15 lignes de test.

### Phase 2 — Première jonction · J10–J16 · 8–14 août

| Toi — parsing | Binôme — exécution |
|---|---|
| Le parser : tokens → structs de commande | Pipes à n commandes |
| Détection des erreurs de syntaxe, `exit 2` | Redirections `<` `>` `>>`, appliquées **dans l'ordre** |
| Abandon propre en cours de construction : libérer un arbre à moitié bâti | Hygiène des descripteurs, boucle de `waitpid`, `$?` |

**Vers J13 :** première vraie jonction — ton parser alimente son exécuteur. Prévoir 1 à 2 jours de
frottement. **C'est planifié, ce n'est pas un échec.**

> **Gate J16** — `cat f | grep x > out` fonctionne depuis un vrai parsing. Si tu es encore sur le lexer
> ici : ~1 semaine de retard, bonus abandonné définitivement.

### Phase 3 — Les morceaux durs · J17–J23 · 15–21 août

| Toi — parsing | Binôme — exécution |
|---|---|
| Expansion de `$VAR` et `$?` | Les 7 builtins, stockage de l'env |
| Word-splitting **uniquement** sur le résultat non quoté | Signaux : interactif, enfant, et pendant un heredoc |
| Suppression des quotes, en toute dernière étape | Double chemin : builtin seul → dans le père ; builtin dans un pipe → dans le fils |
| Parsing du heredoc, expansion dedans selon que le délimiteur était quoté ou non | Application des redirections **commune** aux builtins et aux externes |

> **Gate J23** — Toute la liste du mandatory passe sur le fichier de tests. **Décision bonus ici, et
> uniquement ici.** Si tu refactores encore les quotes à ce stade : zone rouge, stop features.

### Phase 4 — Chasse · J24–J30 · 22–28 août

Ce n'est pas du rembourrage. Les fonctionnalités ne rattrapent jamais un crash ni une fuite.

- `valgrind --leak-check=full --track-fds=yes --trace-children=yes` sur les 25 cas de test
- Norminette clean sur 100 % des fichiers, bonus inclus
- **Relecture croisée complète** : chacun lit l'intégralité de la moitié de l'autre. Non négociable —
  vous êtes interrogés tous les deux, et la modification demandée peut tomber sur n'importe quelle moitié
- Diff systématique contre bash sur les cas limites
- README (voir [§8](#8-checklist-avant-de-geler-le-code))
- **Réserver les soutenances dès J28**, pas à J31

> **Gate J30** — Code gelé et poussé. Aucune fonctionnalité ajoutée après cette date.

### Phase 5 — Soutenances + Deepthought · J31–J40 · 29 août – 7 sept

Fenêtre entièrement consacrée à la validation. C'est toute la raison pour laquelle le code est fini à J30
et pas à J38.

---

## 5. Répartition et territoires partagés

La charge n'est pas 50/50 : le parsing représente ~60 % du temps, l'exécution est plus courte mais bien
plus dense en bugs vicieux. Le déséquilibre se corrige en donnant à l'exécution tout ce qui l'entoure.

| Zone | Propriétaire | Note |
|---|---|---|
| Lexer, parser, expansion | **Toi** | Le gros morceau. ~60 % du budget total |
| fork / exec / pipes / redirections | **Binôme** | Court mais piégeux : les fuites de descripteurs bloquent le shell sans message |
| Builtins (7) | **Binôme** | Rééquilibrage de charge |
| Signaux | **Binôme** | Une seule globale autorisée, contenant uniquement le numéro de signal |
| Table d'environnement | **Partagé** | Propriétaire à désigner en Phase 0. Ton expansion la lit, ses builtins l'écrivent |
| Heredoc | **Partagé** | Toi : détection et lecture. Lui : branchement du descripteur |
| Codes de sortie | **Partagé** | Ton parser produit le 2 (syntaxe), son exécuteur les 126/127/130/131 |
| Makefile, libft | **Un seul des deux** | Deux propriétaires = conflits de merge permanents |

---

## 6. Signaux d'alarme

À vérifier à chaque gate.

| Symptôme | Diagnostic | Réaction immédiate |
|---|---|---|
| À J9, l'exécution attend ton parser | Le mock n'a pas été fait — vous êtes en série | Il code la struct en dur aujourd'hui, avant tout le reste |
| À J16, tu es encore sur le lexer | ≈ 1 semaine de retard | Bonus abandonné définitivement. On protège la Phase 4 |
| À J23, tu refactores les quotes | Zone rouge — le design de Phase 0 était faux | Stop features. On stabilise ce qui marche |
| Le shell se bloque sans message sur un pipe | Descripteur non fermé : le lecteur ne voit jamais l'EOF | Audit : chaque processus ferme les deux extrémités qu'il n'utilise pas |
| Vous n'avez pas relu le code de l'autre à J28 | Risque direct en soutenance | Une demi-journée chacun, toutes affaires cessantes |
| Vous ajoutez une fonctionnalité après J30 | Le code n'est pas gelé | Refuser |

---

## 7. Le contrat des quatre structs — livrable de Phase 0

Pour chacune, la question qui compte n'est pas « quels champs » mais **« qui la libère »**.

| Struct | Produite par → consommée par | Doit porter | Durée de vie |
|---|---|---|---|
| **Token** | Lexer → parser | Sa nature (mot ou opérateur, et lequel). Pour un mot : ses morceaux, chacun avec son type de quote | La plus courte. Meurt dès que le parser a fini |
| **Commande** | Parser → exécuteur | Un tableau d'arguments (case 0 = la commande, terminé par `NULL`) et une liste ordonnée de redirections typées | Jusqu'à la fin de l'exécution de la ligne |
| **Nœud** | Parser → exécuteur | Un discriminant, et ses enfants | Idem commande. Se libère récursivement |
| **État du shell** | Vit à côté de l'arbre | La table d'environnement, le dernier code de sortie. Passé en paramètre, **jamais** en global | Jusqu'à `exit` |

### Ce qu'il faut écrire, pour chacune des quatre

1. **Quels compartiments** elle contient (un texte ? un entier ? une flèche ? un tableau ?)
2. **Qui l'alloue**
3. **Qui la libère**, et quand
4. **Combien de temps elle vit** — meurt-elle à la fin du parsing, à la fin de la ligne, ou à la fin du shell ?

### ⚠ Le piège que Python cachait

Un dict Python disparaît tout seul. En C, **chaque struct allouée est une fuite en attente**.

Le réflexe à prendre dès la Phase 0 : **pour chaque struct définie, écrire sa fonction de libération dans
la même heure.** Trois raisons :

- une struct imbriquée se libère récursivement, et on oublie toujours un niveau si on le fait après coup ;
- ton parser doit pouvoir **abandonner sur une erreur de syntaxe** et libérer un arbre à moitié construit.
  Si la fonction de libération n'existe pas, tu écriras un `return` qui fuit, et il y en aura vingt ;
- ça force à trancher tout de suite qui possède les chaînes de caractères.

**Ne jamais faire pointer une commande vers du texte appartenant à un token** : leurs durées de vie
diffèrent, c'est la source de double-free la plus fréquente du projet.

---

## 8. Checklist avant de geler le code

| Domaine | À vérifier |
|---|---|
| **Codes de sortie** | 127 introuvable · 126 permission ou répertoire · 1 erreur de redirection · 2 erreur de syntaxe · 130 ctrl-C · 131 ctrl-\ · `exit` masqué modulo 256 · `exit abc` ne crashe pas |
| **Messages d'erreur** | En anglais, format bash, sur **stderr** — pas sur stdout |
| **Signaux interactifs** | ctrl-C → nouveau prompt sur une nouvelle ligne · ctrl-D → quitte · ctrl-\ → rien · comportement différent pendant un heredoc |
| **Quotes** | `echo "a\|b"` · `echo ">"` · `a"b c"d` = un seul mot · `'$A'` non expandé · `"$A"` = 1 arg vs `$A` = n args · quote non fermée → erreur de syntaxe |
| **Redirections** | `> a > b` crée les deux · `< f1 < f2` ouvre les deux · `> out echo hi` fonctionne · plusieurs heredocs dans un pipeline |
| **Builtins** | `cd` et `export` seuls persistent · dans un pipe, ils ne modifient pas le shell · `pwd > f` écrit bien dans le fichier |
| **Mémoire** | valgrind clean sur les 25 cas, descripteurs inclus. Les fuites de `readline` sont tolérées ; les vôtres, non |
| **Norme** | 25 lignes par fonction · 5 fonctions par fichier · 4 paramètres · bonus inclus dans le check |
| **README** | Première ligne en italique au format imposé · sections Description, Instructions, Resources · **description de l'usage de l'IA** · rédigé en anglais |
| **Makefile** | `$(NAME) all clean fclean re` · `-Wall -Wextra -Werror` · pas de relink inutile · compile libft via son propre Makefile |

---
---

# Partie II — Le cours

## 9. Lexer et parser : la frontière

Quand tu lis une phrase, ton cerveau fait deux choses distinctes : il regroupe les lettres en **mots**,
puis il comprend la **structure** de la phrase. Un shell fait pareil, et ce sont deux programmes séparés.

- Le **lexer** découpe une suite de caractères en unités atomiques appelées **tokens**. Il ne comprend
  rien à ce qu'il découpe.
- Le **parser** reçoit la liste de tokens et en déduit une structure exploitable.

« Lexer », « tokenizer » et « scanner » désignent la même chose. Vous en avez déjà un : `tokenizer.py`.
Et `parser.py` est votre parser.

### Ce que produit un lexer

Entrée — une seule chaîne brute, telle que readline la donne :

```
cat "mon fichier.txt" | grep -n hello >> out
```

Sortie — une liste de tokens :

```
[mot: cat] [mot: mon fichier.txt] [op: |] [mot: grep] [mot: -n] [mot: hello] [op: >>] [mot: out]
```

Dans un shell il n'existe que **deux familles de tokens** : les **mots** et les **opérateurs**
(`|`, `<`, `>`, `>>`, `<<`). Rien d'autre.

Remarque ce que le lexer a fait et n'a pas fait :

- il a **collé** `mon fichier.txt` en un seul token malgré l'espace, parce que les guillemets le disaient ;
- il a **séparé** `hello` de `>>` alors qu'il n'y a pas d'espace entre eux ;
- il n'a **aucune idée** que `cat` est une commande ou que `out` est un fichier de sortie. C'est le
  travail du parser.

### Un token valide peut être inadmissible

Exemple : `&&` dans la version mandatory.

- **Le lexer l'accepte.** Lexicalement, c'est un opérateur bien formé. Son travail est de découper, pas de
  juger.
- **Le parser le refuse.** Il déroule sa grammaire et rencontre un token pour lequel aucune règle
  n'existe. Il s'arrête, produit l'erreur, libère ce qu'il avait construit, et renvoie 2.

Le lexer voit la **forme**, le parser voit le **sens**.

### Le bénéfice pratique de la séparation

Tu peux valider ton lexer seul, en affichant ses tokens, avant d'avoir écrit une ligne de parser. C'est
exactement le Gate J9. Un bug de découpage se voit en 2 secondes dans une liste de tokens, et en 2 heures
dans un exécuteur qui se comporte bizarrement.

---

## 10. Le pré-traitement, une impasse

Un **pré-traitement**, c'est modifier la ligne *avant* de l'analyser : un premier passage qui réécrit le
texte, puis un second qui travaille sur le texte réécrit.

C'est ce que fait `tokenizer.py:26-27` :

```
ligne 26 :  on remplace tous les |  par  " | "
ligne 27 :  on met des espaces autour de tous les > >> < ;
```

L'idée est séduisante : « si je mets des espaces partout autour des opérateurs, il n'y aura plus qu'à
découper sur les espaces ». Et ça marche sur les cas simples.

### Le problème : ce passage est aveugle

Il remplace **tous** les `|` sans distinction. Il n'a aucun moyen de savoir si un `|` donné est un
opérateur ou une lettre à l'intérieur d'une chaîne entre guillemets — parce qu'à ce moment-là, personne
n'a encore regardé les quotes.

```
étape 1 (aveugle)   :   echo "a|b"    →    echo "a | b"
étape 2 (découpage) :                  →    [ "echo" , "a | b" ]
                                                       ▲
                                        deux espaces qui n'existaient pas
```

Les données sont corrompues, et c'est irréparable : rien ne distingue ces espaces des vrais.

### L'approche correcte : un seul passage, avec mémoire

Tu parcours la ligne **caractère par caractère, une seule fois**, en te souvenant en permanence d'une
chose : *suis-je à l'intérieur d'une quote, et de laquelle ?* Ce souvenir est l'**état**. Il n'a que trois
valeurs : hors quote, simple quote, double quote.

Déroulé sur `echo "a|b"` :

| caractère | état courant | décision |
|---|---|---|
| `e` `c` `h` `o` | hors quote | lettres ordinaires → on accumule le mot `echo` |
| espace | hors quote | séparateur → le mot `echo` est fini, token produit |
| `"` | hors quote | → **changement d'état** : on entre en double quote. Le `"` n'est pas gardé |
| `a` | double quote | on accumule → `a` |
| `\|` | double quote | **ici, ce n'est PAS un opérateur.** On accumule → `a\|` |
| `b` | double quote | on accumule → `a\|b` |
| `"` | double quote | → **changement d'état** : on sort. Le `"` n'est pas gardé |
| fin de ligne | hors quote | le mot est fini → token `a\|b` |

Le même caractère `|` aurait été traité **différemment** en état « hors quote ». La décision dépend de
l'état, et l'état n'existe que parce qu'on avance dans l'ordre. Un remplacement global n'a pas d'état — il
ne peut pas avoir raison.

### L'exemple qui prouve que ce n'est pas rattrapable

```
a"b c"d
```

Bash y voit **un seul mot** : `ab cd`.

Aucune stratégie « j'insère des espaces puis je découpe sur les espaces » ne peut produire ça, parce que
**la frontière du mot n'est pas déterminée par les espaces**. Il y a un espace au milieu du mot, et pas
d'espace autour des quotes.

Ce n'est donc pas une regex à améliorer, c'est une approche à abandonner. L'information manquante — *suis-je
dans une quote ?* — ne peut s'obtenir qu'en balayant séquentiellement.

### Bénéfice annexe du passage unique

Pour distinguer `>` de `>>`, tu dois regarder le caractère suivant. En avançant caractère par caractère,
tu l'as sous la main naturellement. Le pré-traitement rendait ça fragile : l'ordre `>>|>` dans votre regex
était obligatoire, sinon `>>` se cassait en deux `>`.

---

## 11. Les quotes : le problème central

**Le lexer est le seul endroit du programme qui voit jamais les caractères `'` et `"`.** Après lui, ils
n'existent plus — c'est lui qui les retire, c'est son travail. Donc **tout ce que le lexer ne note pas au
passage est perdu définitivement**.

Or l'expansion (`$VAR`), qui vient *après*, a besoin de savoir si un morceau était quoté et comment :

| ligne | ce que le lexer voit | texte du token produit |
|---|---|---|
| `echo "$A"` | doubles quotes autour de `$A` | `$A` |
| `echo $A` | pas de quotes | `$A` |
| `echo '$A'` | simples quotes autour de `$A` | `$A` |

**Les trois tokens ont le même texte.** Et pourtant les trois doivent se comporter différemment : le
premier s'expand sans re-découpage, le deuxième s'expand avec re-découpage, le troisième ne s'expand pas
du tout.

Conclusion : **un token de shell ne peut pas être juste une chaîne de caractères.** Il doit transporter
l'information de quote. D'où la décision gelée : le token est une struct qui porte ses morceaux, chacun
avec son type de quote.

### L'ordre correct des étapes

Bash procède ainsi, et il faut le respecter :

1. découper en mots **en respectant les quotes**
2. expandre les variables
3. re-découper en mots **uniquement le résultat non quoté**
4. **supprimer les quotes** — en tout dernier

Votre code Python fait l'inverse : `shlex.split` supprime les quotes à la ligne 29, et `expand_var` passe
à la ligne 32. D'où le contournement `__NO_EXPAND__` (`tokenizer.py:9-14`) — qui est cassable : tapez
`echo __NO_EXPAND__$HOME`.

### Le mécanisme retenu, et pourquoi

Trois options existaient :

| Option | Principe | Verdict |
|---|---|---|
| **Mot = liste de morceaux** | Un mot est une suite de morceaux, chacun portant son type de quote | **Retenue.** Le plus fidèle à bash, le plus facile à débugger et à défendre en soutenance |
| Masque parallèle | Le mot reste une chaîne + un tableau d'octets de même longueur disant si chaque caractère était quoté | Compact, mais toute l'expansion devient de l'arithmétique d'indices sur deux tampons à synchroniser |
| Octets sentinelles | Encadrer les zones quotées par des octets non imprimables, les retirer à la fin | Le moins de code, mais débogage pénible et il faut prouver que l'utilisateur ne peut pas injecter les marqueurs |

L'option retenue est aussi la seule cohérente avec votre méthode « on stocke dans une struct » : les deux
autres cachent l'information *dans* la chaîne ou *à côté* d'elle, au lieu de lui donner un compartiment.

---

## 12. Structures de données en C : tableau, liste, arbre

Le C ne fournit **ni liste, ni arbre**. Il n'y a pas de type « arbre » en C. Il donne exactement deux
briques, et tout le reste est construit avec elles :

1. **La struct** — une boîte avec des compartiments nommés
2. **Le pointeur** — une adresse. Un compartiment qui ne contient pas une valeur, mais **l'endroit où se
   trouve une autre boîte**

Liste, arbre, pile, table de hachage : ce sont des **conventions d'assemblage** de boîtes et de flèches.
Elles n'existent pas dans le langage, elles existent dans la façon dont tu branches tes pointeurs.

C'est ce que Python cachait : une `list` Python fait tableau, liste chaînée et redimensionnement d'un coup,
et un `dict` contient n'importe quoi. En C, tu dois choisir.

### Le tableau — des boîtes collées, numérotées

```
┌─────────┬─────────┬─────────┬─────────┐
│    0    │    1    │    2    │    3    │
└─────────┴─────────┴─────────┴─────────┘
 ◄──────── un seul bloc de mémoire ────────►
```

Toutes les cases sont **contiguës**, de taille identique. Accès direct à la case 2 : le programme calcule
son adresse. Un seul `malloc` pour tout le bloc.

**Le prix :** tu dois connaître le nombre de cases au moment où tu alloues. Pour en ajouter une, il faut
allouer un bloc plus grand ailleurs et recopier.

### La liste chaînée — des boîtes éparpillées, reliées

```
┌──────────┐     ┌──────────┐     ┌──────────┐
│ contenu  │     │ contenu  │     │ contenu  │
│ suivant ●┼────►│ suivant ●┼────►│ suivant ∅│
└──────────┘     └──────────┘     └──────────┘
                                        ∅ = NULL = fin
```

Chaque boîte est un `malloc` séparé, posée n'importe où. Elle contient son contenu **plus un compartiment
qui dit où est la suivante**. La dernière contient `NULL`.

**L'avantage :** tu n'as pas besoin de savoir combien il y en aura.
**Le prix :** pour atteindre la 5ᵉ, tu pars de la 1ʳᵉ et tu suis 4 flèches.

### L'arbre — une liste chaînée qui peut bifurquer

> **Un arbre, c'est une liste chaînée où chaque boîte a deux compartiments de flèche au lieu d'un.**

```
liste :   boîte ──► boîte ──► boîte ──► ∅       (une flèche par boîte)

arbre :        boîte
              ╱     ╲                           (deux flèches par boîte)
         boîte       boîte
        ╱     ╲
   boîte       boîte
```

**Aucune** nouvelle notion : même struct, même `malloc`, même `NULL` pour dire « rien ici ». Juste un
deuxième pointeur. Une liste est un arbre qui ne bifurque jamais ; un arbre est une liste qui peut
bifurquer.

Une boîte dont les deux flèches sont à `NULL` s'appelle une **feuille**. La boîte du haut s'appelle la
**racine**.

### Les trois servent dans ton minishell

| Où | Structure | Pourquoi celle-là |
|---|---|---|
| **`argv`** | **Tableau** de pointeurs vers des chaînes, terminé par `NULL` | Pas un choix : `execve` **exige** cette forme |
| **Les redirections** d'une commande | **Liste chaînée** | Tu les découvres une par une, sans savoir combien. Et l'ordre compte |
| **Les morceaux quotés** d'un token | **Liste chaînée** | Même raison : `a"b c"d` fait trois morceaux |
| **La structure de la ligne** | **Arbre** | Voir [§13](#13-la-structure-de-la-ligne--liste-plate-ou-arbre) |

`argv` est d'ailleurs un **tableau de tableaux** : un tableau de pointeurs dont chacun mène à une chaîne,
et une chaîne est elle-même un tableau.

### Le rôle de `NULL`

`NULL` fait tout le travail silencieusement : « fin de liste », « pas d'enfant ici », « fin de `argv` ».
Toutes tes fonctions récursives s'arrêtent en testant `NULL` — sans lui, elles ne sauraient jamais quand
cesser de descendre.

En Python, la fin d'une liste est connue parce que la liste connaît sa longueur. En C, la fin est une
valeur convenue *à l'intérieur* de la structure. C'est aussi pourquoi oublier le `NULL` final d'`argv`
fait lire `execve` dans de la mémoire au hasard.

---

## 13. La structure de la ligne : liste plate ou arbre

### Attention : trois sens du mot « liste »

| Expression | De quoi ça parle | Ce que ça veut dire |
|---|---|---|
| **liste chaînée** | Le **mécanisme C** | Des boîtes reliées par une flèche chacune. C'est l'outil de construction |
| **liste ordonnée** (redirections) | Une **exigence de contenu** | « L'ordre d'apparition doit être conservé ». Se réalise avec une liste chaînée |
| **liste plate** (structure de la ligne) | Une **forme** | Un seul niveau de profondeur, aucune imbrication. L'opposé de l'arbre |

### La liste plate — votre design Python actuel

```
liste plate :          pipeline
                      ╱    │    ╲
                   cmd    cmd    cmd
```

Toujours exactement deux niveaux : le conteneur, et des commandes. Aucune de ces commandes ne peut
elle-même contenir un pipeline. « Plate » parce qu'elle n'a aucun relief : c'est toujours le même unique
étage, quelle que soit la ligne.

Pour `ls -l | grep txt | wc -l`, c'est **parfaitement suffisant**. Un pipeline n'a ni priorité ni
imbrication.

### L'arbre — profondeur variable

```
arbre :                  ou
                       ╱     ╲
                     et       pipeline
                   ╱    ╲
            pipeline   pipeline
```

La profondeur dépend de la ligne : un étage pour `ls | wc`, trois pour `a && b || c`.

Le nom savant de cette structure est **AST** — *abstract syntax tree*, arbre syntaxique abstrait.
« Abstrait » parce qu'il ne garde que la structure logique et jette la ponctuation : **les parenthèses
disparaissent**. Elles n'ont servi qu'à décider où l'arbre se branche. C'est le même phénomène que les
quotes dans le lexer : un caractère qui influence la structure puis s'évapore.

### Pourquoi l'exécution devient récursive

Pour exécuter un nœud `&&`, il faut exécuter le sous-arbre de gauche, **récupérer son code de sortie**, et
décider seulement ensuite s'il faut exécuter celui de droite. Or « exécuter le sous-arbre de gauche » peut
vouloir dire exécuter un pipeline entier, ou un autre `&&`. L'exécuteur s'appelle donc lui-même sur ses
enfants, jusqu'à tomber sur des feuilles.

Une liste plate n'offre aucun endroit où accrocher « le résultat de tout ce bloc-là ».

Le parser et l'exécuteur deviennent des miroirs : le parser descend récursivement dans la grammaire pour
**construire** l'arbre, l'exécuteur descend récursivement dans l'arbre pour l'**évaluer**.

### Le pipeline reste n-aire — et c'est important

Pour `ls | wc | cat`, deux designs sont possibles :

```
pipe binaire (2 enfants)         pipeline n-aire (une liste)

        pipe                            pipeline
       ╱    ╲                          ╱    │    ╲
    pipe     cat                     ls    wc     cat
   ╱    ╲
 ls      wc

    5 boîtes                            4 boîtes
```

**Le second est nettement meilleur**, pour une raison d'exécution : il faut **forker toutes les commandes
du pipeline avant d'attendre la première**, sinon blocage silencieux (voir
[§17](#17-fork-execve-et-pourquoi-pas-de-threads)). Avec des pipes binaires imbriqués, la récursion veut
naturellement évaluer complètement la gauche avant de toucher la droite — exactement le comportement qui
provoque le blocage. Avec un pipeline qui contient la liste de ses commandes, elles sont toutes
accessibles d'un coup.

C'est aussi ce que fait bash : sa grammaire définit le pipeline comme une **suite** de commandes, tandis
que `&&` et `||` sont des opérateurs **binaires** entre pipelines. Votre `parse_pipeline` avait déjà raison.

**Règle générale :** la forme de la structure doit épouser l'ordre d'exécution. Le `&&` évalue la gauche
*puis s'arrête pour réfléchir* → deux enfants séquentiels. Le pipeline lance tous ses enfants *en même
temps* → tous accessibles d'un coup.

### Ce que l'arbre coûte, honnêtement

**Pour le mandatory seul, l'arbre ne sert à rien.** Il n'y a qu'un pipeline par ligne, aucune imbrication
possible. La liste plate est suffisante et plus simple.

L'arbre est une **assurance** :

- l'exécuteur reçoit « un nœud » au lieu de « un pipeline » → une fonction d'entrée qui regarde le
  discriminant et redirige
- le parser gagne un niveau au-dessus de `parse_pipeline`, qui pour le mandatory ne fait rien d'autre que
  retourner le pipeline
- la struct de nœud est un peu plus large

**Coût : 3 à 5 heures en Phase 0.** Coût de la conversion inverse au jour 25, avec un exécuteur déjà écrit
contre la liste : **2 à 3 jours** — que vous n'avez pas.

C'est une prime d'assurance à 4 heures contre un sinistre à 3 jours.

---

## 14. `argv` : un seul tableau

Aujourd'hui, votre dict a **deux compartiments séparés** (`parser.py:5-6`) :

```
cmd  ──► "grep"
args ──► [ "f" ]
```

Ce qu'il faut : **un seul compartiment**, un tableau unique, où la case 0 est le nom de la commande.

```
argv ──► [ "grep" , "f" , NULL ]
            ▲
            └─ case 0 = la commande. Pas de champ séparé.
```

### Trois raisons

**a) `execve` l'exige.** Cette fonction ne prend pas « un nom + des arguments ». Elle prend **un seul
tableau contigu, terminé par `NULL`**, dont la case 0 est le nom du programme. Si tu stockes en deux
morceaux, il faut en fabriquer un troisième juste avant l'exec — c'est ce que fait
`executor_simple.py:27`. En C, ça veut dire un `malloc` supplémentaire, dans l'enfant, à l'endroit où la
gestion d'erreur est la plus pénible.

**b) Ça supprime votre bug.** Le compartiment séparé vous a forcés à écrire `parser.py:48` : « si ce token
est égal au nom de la commande, ignore-le ». Résultat mesuré : `grep grep f` perd un argument.

Avec un tableau unique, tu parcours les tokens de gauche à droite et **tu empiles tout ce qui n'est pas
une redirection**. Le premier empilé se trouve en case 0, donc c'est la commande. Rien à comparer.

**c) Ça règle le cas tordu gratuitement.** `> out echo hi` vous obligeait aux acrobaties d'indices de
`parser.py:17-25`. Avec le parcours unique : le `>` consomme `out` comme cible, puis `echo` s'empile
(case 0), puis `hi` (case 1). Zéro cas particulier.

### ⚠ Le `NULL` final

`execve` ne connaît pas la longueur du tableau ; il avance jusqu'à trouver `NULL`. Si tu l'oublies, il
continue à lire de la mémoire au hasard. Pour 2 arguments, tu alloues **3** cases.

---

## 15. Les redirections : une liste ordonnée

**Ordonnée = dans l'ordre où l'utilisateur les a tapées, de gauche à droite.** Pas triées par type, pas
triées par nom. Juste : on garde l'ordre de la ligne.

### Déroulé complet sur `echo hi > a > b`

**Votre design actuel — deux cases écrasables** (`parser.py:8-9`) :

| étape | ce qu'il lit | contenu de la case `stdout` |
|---|---|---|
| 1 | `> a` | `a` |
| 2 | `> b` | `b` ← **`a` est écrasé, il a disparu** |

Conséquence observable : après la commande, `ls` ne montre **pas** de fichier `a`. Dans bash, il existe (vide).

**Le design en liste ordonnée :**

| étape | ce qu'il lit | contenu de la liste |
|---|---|---|
| 1 | `> a` | [ (`>`, `a`) ] |
| 2 | `> b` | [ (`>`, `a`) , (`>`, `b`) ] ← les deux sont là |

```
redirections ──► ┌───────────┐   ┌───────────┐   ┌───────────┐
                 │ type : >  │   │ type : >> │   │ type : <  │
                 │ cible: a  │──►│ cible: b  │──►│ cible: c  │──► ∅
                 └───────────┘   └───────────┘   └───────────┘
                     1ère            2ème            3ème
```

Puis l'exécuteur parcourt la liste **dans l'ordre** et applique chaque boîte bêtement :

| étape | action | état de la sortie |
|---|---|---|
| 1 | ouvre `a` (le crée, le vide), branche la sortie dessus | → `a` |
| 2 | ouvre `b` (le crée, le vide), branche la sortie dessus | → `b` |

Résultat : `a` **et** `b` existent, `a` est vide, `hi` part dans `b`. **C'est exactement bash.**

### Le point élégant

Pour décider que c'est `b` qui gagne, l'exécuteur n'a comparé personne et testé aucune priorité. Il a
appliqué les redirections l'une après l'autre, et la deuxième a naturellement remplacé la première.

> **L'ordre d'application *est* la règle.** Tu n'as pas de logique à écrire, tu as juste à ne pas perdre
> l'ordre.

### Deux autres cas que seule la liste permet

- `cat < f1 < f2` — bash ouvre **les deux**. Si `f1` n'existe pas, tu dois échouer même si `f2` existe.
  Avec une case écrasée, tu ne sais même pas que `f1` a été demandé.
- `cat << A << B` — deux heredocs. Il n'y a **aucun endroit** pour ranger le second dans votre design.

Le heredoc est d'ailleurs un **quatrième type** de redirection, qui coexiste avec les autres. C'est la
seule forme où il a une place.

---

## 16. `&&` et `||`

### Cas A — mandatory : quelqu'un tape `&&` et vous ne l'implémentez pas

Situation réelle de la Phase 2, souvent oubliée. Le `&&` n'est pas demandé, mais **un correcteur le
tapera**.

Bonne réponse : **erreur de syntaxe, message sur stderr, `$?` à 2.**

```
minishell: syntax error near unexpected token `&&'
```

Le sujet l'autorise explicitement : « Not interpret [...] special characters which are not required by the
subject ».

**L'erreur à éviter :** traiter `&` comme un caractère ordinaire. `ls && wc` donnerait
`argv = ["ls", "&&", "wc"]` — donc `ls` avec deux arguments bizarres. Bash ne fait jamais ça.

Ton lexer doit donc **savoir reconnaître `&&`** (et `&` seul) même en version mandatory. Ça coûte trois
lignes, et c'est ce qui permet un message d'erreur propre. C'est le parser qui refuse, pas le lexer
(voir [§9](#9-lexer-et-parser--la-frontière)).

### Cas B — bonus : la sémantique du court-circuit

Pour exécuter un nœud **ET** :

1. exécute le sous-arbre de gauche, récupère son code de sortie
2. si ce code vaut **0** (succès) → exécute la droite ; le résultat du ET est le code de droite
3. si ce code est **non nul** (échec) → **n'exécute pas** la droite ; le résultat du ET est le code de gauche

Le **OU** est le miroir exact : on exécute la droite seulement si la gauche a **échoué**.

On appelle ça le *court-circuit* : le deuxième opérande n'est évalué que si nécessaire.

### ⚠ Les deux pièges de priorité

**1. `&&` et `||` ont la MÊME priorité**, et s'associent de gauche à droite. Ce n'est **pas** comme en C.

```
a || b && c        se lit        (a || b) && c
```

**2. Le `|` est PLUS fort que `&&` et `||`.** Les pipelines se forment d'abord, les nœuds ET/OU se posent
par-dessus. C'est pour ça que les PIPELINE sont **en bas** de l'arbre et les ET/OU **en haut**.

Sur `make && ls | wc || echo ko` :

```
                      ┌────┐
                      │ OU │
                      └┬──┬┘
              ┌────────┘  └────────┐
              ▼                    ▼
           ┌────┐            ┌──────────────────┐
           │ ET │            │ PIPELINE         │
           └┬──┬┘            │  └─ echo ko      │
       ┌────┘  └────┐        └──────────────────┘
       ▼            ▼
┌────────────┐  ┌──────────────────┐
│ PIPELINE   │  │ PIPELINE         │
│  └─ make   │  │  ├─ ls           │
└────────────┘  │  └─ wc           │
                └──────────────────┘
```

Trois pipelines, deux opérateurs. Le `ls | wc` reste **groupé dans un seul pipeline**, jamais coupé par
les opérateurs qui l'entourent.

### Ce que ça change dans le contrat : rien

| | Mandatory | Bonus |
|---|---|---|
| Ton lexer | reconnaît `&&` | reconnaît `&&` — **identique** |
| Ta struct de nœud | prévoit le type ET/OU | prévoit le type ET/OU — **identique** |
| Ton parser | **refuse** le token, renvoie 2 | **construit** le nœud |
| Ton exécuteur | ne rencontre jamais de nœud ET/OU | sait descendre dedans |

Les deux versions diffèrent par **un seul endroit**. Tu n'écris pas le bonus, tu écris la **place** du
bonus — et cette place te sert dès le mandatory à produire une vraie erreur de syntaxe.

---

## 17. fork, execve, et pourquoi pas de threads

### Les threads sont interdits

La liste des fonctions autorisées (chapitre IV du sujet) contient `fork`, `wait`, `waitpid`, `wait3`,
`wait4`, `execve`, `pipe`, `dup`, `dup2`, `kill`, `signal`, `sigaction`…

**Aucun `pthread_*`.** Ni `clone`, ni `posix_spawn`. Un thread = fonction externe non autorisée = 0.

Pour la même raison : **pas de `system()` ni de `popen()`**. Le sujet veut que vous écriviez vous-mêmes ce
que ces fonctions cachent.

### Et même autorisés, ils ne marcheraient pas

> **`execve` ne crée rien. Il remplace le programme en cours par un autre.**

Quand tu appelles `execve("/bin/ls", …)`, ton processus **cesse d'être ton shell** : code, variables,
pile, tout est écrasé par `ls`. Il n'y a pas de retour. Si `execve` réussit, la ligne suivante de ton code
n'est jamais atteinte.

D'où la question : comment lancer `ls` sans détruire ton shell ? Réponse : tu fais d'abord une **copie
sacrifiable de toi-même**, et c'est elle que tu laisses écraser. Cette copie, c'est `fork`.

Un thread ne peut pas jouer ce rôle : **les threads d'un processus partagent le même espace mémoire**. Il
n'existe pas de « remplacer le programme de ce thread-là uniquement ». `execve` opère sur le processus
entier — appelé depuis un thread, il rase tout, ton shell avec. Un thread n'est pas sacrifiable, il est
solidaire.

### Processus contre thread, sur ce projet

| Ce dont ton shell a besoin | Processus (`fork`) | Thread |
|---|---|---|
| Remplacer le programme par la commande | Oui, `execve` sur l'enfant | Impossible — détruirait le shell |
| Survivre au crash d'une commande | Oui, un segfault dans l'enfant ne te touche pas | Non, tue tout le processus |
| Survivre à un `exit` de la commande | Oui | Non, termine tout |
| Que `cd` dans un pipe **ne** change **pas** le shell | Oui, mémoire séparée | Non — ça le changerait |
| Table de descripteurs indépendante après `dup2` | Oui | Non, partagée |

La quatrième ligne est importante : la raison pour laquelle `cd /tmp | ls` ne doit **pas** changer le
répertoire de ton shell, c'est précisément l'isolation entre processus. Ce n'est pas une contrainte, c'est
un comportement de bash que `fork` fournit gratuitement.

### Le parallélisme du pipeline

Dans `cat gros_fichier | grep x`, les deux commandes tournent **réellement en même temps**. Mais c'est du
**parallélisme de processus**, et `fork` te le donne déjà : tu crées le pipe, tu forkes, le noyau
ordonnance. Les threads n'apportent rien.

### ⚠ Le piège du tampon de pipe

Le tampon d'un pipe est limité (64 Ko sous Linux). Si tu lances `cat`, que tu **attends qu'il finisse**,
et seulement ensuite que tu lances `grep`, alors `cat` se bloque dès que le tampon est plein — il attend un
lecteur qui n'existe pas encore. **Blocage définitif, sans aucun message d'erreur.**

La solution n'a rien à voir avec les threads : **forker toutes les commandes du pipeline d'abord,
n'attendre qu'ensuite.** Le lecteur existe donc dès le départ.

C'est aussi la raison pour laquelle le pipeline est n-aire dans notre design
([§13](#13-la-structure-de-la-ligne--liste-plate-ou-arbre)).

### Deux points annexes

- `main.py:9` contient `import threading`, et `background.py` fait **0 ligne**. Import mort, à jeter.
- La règle « une seule variable globale, contenant uniquement le numéro de signal » **présuppose un
  programme mono-thread**. Un gestionnaire de signal ne peut faire presque aucun appel de fonction en
  sécurité, et avec plusieurs threads on ne sait même pas lequel reçoit le signal.

### La fenêtre entre `fork` et `execve`

`fork` est appelé **une fois** et retourne **deux fois**, dans deux processus différents, avec une valeur
de retour différente dans chacun. C'est cette valeur qui te dit qui tu es — parent ou enfant. C'est la
question la plus posée en soutenance minishell.

Le couple `fork` + `execve` est un choix délibéré d'Unix : au lieu d'un unique appel « lance ce
programme », on sépare « duplique-moi » et « deviens ce programme ». Ça paraît lourd, mais c'est
exactement ce qui rend les redirections possibles : **entre les deux, l'enfant existe et n'est pas encore
la commande** — c'est la fenêtre où il fait ses `dup2` tranquillement. Sans cette séparation, il n'y
aurait pas de shell.

---

## 18. Les codes de sortie

`$?` n'est pas un gadget d'affichage : c'est la **valeur de retour** de chaque nœud de l'arbre, et c'est
elle qui pilote le flot de contrôle. Un shell est un langage de programmation dont l'unique type de donnée
pour les conditions est le code de sortie d'un processus. Si `$?` est faux quelque part, avec le bonus ça
casse le déroulement du programme.

| Code | Signification |
|---|---|
| **0** | succès |
| **1** | erreur générique (échec de redirection, `unset` invalide…) |
| **2** | erreur de syntaxe |
| **126** | trouvé mais pas exécutable (permission refusée, ou c'est un répertoire) |
| **127** | commande introuvable |
| **130** | tué par ctrl-C (SIGINT) |
| **131** | tué par ctrl-\ (SIGQUIT) |

Autres règles :

- `exit` masque son argument **modulo 256** : `exit 300` donne 44
- `exit abc` ne doit **pas** crasher (`handle_builtins.py:29` fait `int(...)` sans garde)
- `$?` d'un pipeline = code de la **dernière** commande du pipeline
- Format des messages, sur **stderr** : `minishell: cmd: command not found`

---
---

# Partie III — En attente

## Livrable bloquant

**Le contrat des quatre structs** ([§7](#7-le-contrat-des-quatre-structs--livrable-de-phase-0)) — token,
commande, nœud, état du shell. Pour chacune : ses compartiments, qui l'alloue, qui la libère, combien de
temps elle vit. En français, en prose ou en tableau, pas besoin de syntaxe C.

**Ce document se signe à deux.** Le binôme ne démarre pas son mock avant — sinon la Phase 1 part en série
au lieu de partir en parallèle, et c'est la semaine que vous n'avez pas.

## Questions ouvertes

| Question | Impact | Échéance |
|---|---|---|
| **Fait-on le bonus ?** (`&&` `\|\|` avec parenthèses, wildcards `*`) | Si « oui » ferme, deux détails de plus à figer dans la struct de nœud. Si « non » ferme, la liste plate suffit | Décision formelle au Gate J23, mais dis-le dès que tu sais |
| **Date exacte du blackhole** | Toutes les dates de ce document sont calées sur ≈ 7 septembre | Dès que possible |

## Points à traiter ensuite

- Les cas limites que la struct de token doit encaisser — à confronter au contrat, une fois qu'il est écrit
- La récursivité, si la notion n'est pas complètement solide
- Le détail de la fenêtre `fork` / `execve`, utile pour la relecture croisée de la Phase 4

---

*Document de travail — à réviser à chaque gate. Version PDF de la Partie I : `ROADMAP_Minishell.pdf`.*
