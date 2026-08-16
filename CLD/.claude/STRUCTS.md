# Minishell — Référence

> **Partie I** — la forme de chaque struct : ses champs, quand ils sont valides,
> à quoi ressemblent les données en mémoire.
> **Partie II** — les concepts shell : expansion, redirections, heredoc, signaux.
> **Partie III** — le contrat de propriété, **à remplir** (livrable de Phase 0, `MINISHELL_GUIDE.md` §7).

---

# Partie I — Les structures

## 1. Le trajet d'une ligne

```
readline()          "echo a\"b c\" > out | wc -l"        une seule chaîne
     │
     ▼
  LEXER             t_token  ──►  t_token  ──► …          découpage, quotes notées
     │
     ▼
  PARSER            t_command ──► t_command               argv + redirections
     │
     ▼
 EXPANSION          $VAR remplacés, quotes retirées       a besoin de t_shell
     │
     ▼
 EXÉCUTION          fork / dup2 / execve                  a besoin de t_shell
```

| Structure | Naît | Meurt |
|---|---|---|
| chaîne readline | étape 1 | après le lexer |
| `t_token` / `t_piece` | lexer | **fin du parsing** |
| `t_command` / `t_redirect` | parser | fin de l'exécution de la ligne |
| `t_env` / `t_shell` | démarrage | `exit` |

> ⚠ Les tokens meurent **avant** les commandes.
> Donc `argv` ne doit **jamais** pointer vers le `content` d'un `t_piece` — il faut copier.
> **Règle retenue : chaque struct est propriétaire exclusive de ses chaînes.**

---

## 2. Les énumérations

### `t_type` — la nature d'un token

| Valeur | Sens | `redir_type` valide ? | `piece` valide ? |
|---|---|---|---|
| `MOT` | un mot | non | **oui** |
| `PIPE` | `\|` | non | non (`NULL`) |
| `REDIRECT` | `<` `>` `<<` `>>` | **oui** | non (`NULL`) |
| `UNEXPECTED_S` | `&` seul | non | non (`NULL`) |
| `UNEXPECTED_D` | `&&` | non | non (`NULL`) |

Les deux `UNEXPECTED_*` sont reconnus par le lexer mais **toujours refusés par le parser** :

```
minishell: syntax error near unexpected token `&&'
```
→ message sur **stderr**, `exit_code` à **2**.

### `t_type_redirect` — quelle redirection

| Valeur | Symbole | Effet |
|---|---|---|
| `R_IN` | `<` | lit depuis un fichier |
| `R_OUT` | `>` | écrit dans un fichier (le crée / le vide) |
| `R_APPEND` | `>>` | écrit à la fin d'un fichier |
| `R_HEREDOC` | `<<` | lit jusqu'à un délimiteur |

### `t_quote` — comment un morceau était protégé

| Valeur | Écrit | `$VAR` expansé ? |
|---|---|---|
| `Q_NONE` | `abc` | oui |
| `Q_SINGLE` | `'abc'` | **non** — tout est littéral |
| `Q_DOUBLE` | `"abc"` | oui |

---

## 3. `t_piece` — un morceau de mot

```c
char            *content;
t_quote          quote;
struct s_piece  *next;
```

Un mot peut être fait de plusieurs morceaux, chacun protégé différemment.
C'est **la seule trace** du passage des quotes : le lexer est le seul endroit du programme
qui voit jamais un `'` ou un `"`.

`a"b c"d` → **un seul token**, **trois morceaux** :

```
content: "a"     content: "b c"    content: "d"
quote:   Q_NONE  quote:   Q_DOUBLE quote:   Q_NONE
next:    ●───────►next:    ●───────►next:    NULL
```

Les guillemets eux-mêmes ne sont **jamais** stockés. Ce sont des instructions de
changement d'état, pas des données.

---

## 4. `t_token` — la sortie du lexer

```c
t_type            type;
t_type_redirect   redir_type;   /* valide seulement si type == REDIRECT */
t_piece          *piece;        /* NULL si type != MOT                  */
struct s_token   *next;
```

Chaînés dans l'ordre de la ligne. Le nombre est inconnu à l'avance — d'où le `next`.

### Comment le lexer découpe

Automate à **trois états** :

| Caractère | `NORMAL` | `IN_SINGLE` | `IN_DOUBLE` |
|---|---|---|---|
| espace / tab | **coupe** | accumule | accumule |
| `\|` `<` `>` `&` | **coupe** + produit un opérateur | accumule | accumule |
| `'` | → `IN_SINGLE` | → `NORMAL` | accumule |
| `"` | → `IN_DOUBLE` | accumule | → `NORMAL` |
| autre | accumule | accumule | accumule |

Deux règles de coupe **indépendantes** :

```
hello>>out    →  [MOT hello] [REDIRECT R_APPEND] [MOT out]     3 tokens, 0 espace
hello   out   →  [MOT hello] [MOT out]                         2 tokens, 3 espaces
```

Le lexer **colle** ce que les espaces séparent (si quoté) et **sépare** ce qui est collé
(si opérateur).

### Pièges

- `>` ou `>>` ? Il faut regarder le caractère **suivant**, puis avancer de 2 crans.
- Fin de ligne en `IN_SINGLE` / `IN_DOUBLE` → **erreur de syntaxe**. L'état final suffit à la détecter.
- `echo ""` doit donner **2 arguments**. Un mot n'est pas vide parce que son contenu l'est,
  mais parce qu'aucun caractère n'a été lu — quotes comprises.
- Un opérateur a `piece == NULL`. La fonction de libération doit le supporter.

---

## 5. `t_redirect` — une redirection

```c
t_type_redirect     type;
char               *target;      /* nom de fichier, ou délimiteur si R_HEREDOC */
int                 flag_quote;  /* n'a de sens que si R_HEREDOC               */
struct s_redirect  *next;
```

**Liste ordonnée** = dans l'ordre où l'utilisateur les a tapées, de gauche à droite.
Pas triées par type, pas triées par nom.

### Pourquoi une liste et pas deux cases

`echo hi > a > b` — bash crée **`a` ET `b`**. `a` reste vide, `hi` part dans `b`.

```
redirs ──► ┌──────────────┐   ┌──────────────┐
           │ type: R_OUT  │──►│ type: R_OUT  │──► NULL
           │ target: "a"  │   │ target: "b"  │
           └──────────────┘   └──────────────┘
```

L'exécuteur applique dans l'ordre, bêtement :

| étape | action | sortie |
|---|---|---|
| 1 | ouvre `a` (le crée, le vide), branche dessus | → `a` |
| 2 | ouvre `b` (le crée, le vide), branche dessus | → `b` |

**L'ordre d'application _est_ la règle.** Aucune priorité à coder, juste ne pas perdre l'ordre.

Deux autres cas que seule la liste permet :

- `cat < f1 < f2` — les deux sont ouverts. Si `f1` n'existe pas, il faut échouer même si `f2` existe.
- `cat << A << B` — deux heredocs. Avec une case unique, aucun endroit pour le second.

### `flag_quote`

```
<< EOF        flag_quote = 0   →  le corps du heredoc EST expansé
<< "EOF"      flag_quote = 1   →  le corps n'est PAS expansé
<< 'EOF'      flag_quote = 1   →  idem
<< E"OF"      flag_quote = 1   →  une seule quote quelque part suffit
```

Dans les quatre cas, `target` vaut `"EOF"`. C'est le **flag** qui fait la différence, pas la cible.

---

## 6. `t_command` — la sortie du parser

```c
char              **argv;      /* case 0 = la commande, terminé par NULL */
t_redirect         *redirs;    /* NULL si aucune redirection             */
struct s_command   *next;      /* NULL si dernière du pipeline           */
```

La chaîne de `t_command` **est** le pipeline. Pas d'arbre : un seul étage.

```
cmd(cat) ──next──► cmd(grep) ──next──► cmd(wc) ──► NULL
```

### `argv` : un seul tableau

```
argv ──► [ "grep" ][ "-n" ][ "hello" ][ NULL ]
             ▲                            ▲
        la commande             execve s'arrête ici
```

Le `NULL` final n'est pas décoratif : c'est la **seule** façon dont `execve` sait où s'arrêter.

### L'exécution est n-aire, jamais imbriquée

Il faut **forker toutes les commandes avant d'attendre la première**.

Contre-exemple : `cat gros_fichier | head -1`. Si on exécute la gauche jusqu'au bout d'abord —
le tampon d'un pipe fait 64 Ko, `cat` le remplit, bloque dans `write()`, et personne ne lit
parce que `head` n'a pas encore été forké. **Blocage définitif et silencieux.**

Le code de sortie d'un pipeline est celui de la **dernière** commande.

---

## 7. `t_env` et `t_shell` — l'état persistant

```c
/* t_env */
char           *key;        /* "USER"                                     */
char           *value;      /* "nico", ou NULL si déclarée sans valeur    */
int             exported;   /* voir tableau ci-dessous                    */
struct s_env   *next;
```

```c
/* t_shell */
t_env       *env;
int          exit_code;     /* ce que renvoie $?                          */
```

### Le champ `exported`

| Commande | `key` | `value` | `exported` | Visible dans `env` | Visible dans `export` |
|---|---|---|---|---|---|
| `export A=10` | `"A"` | `"10"` | 1 | oui | oui |
| `export A` | `"A"` | `NULL` | 1 | **non** | oui |

Seules les variables visibles dans `env` sont passées à `execve`.

### Pourquoi une struct et pas un global

Le sujet impose : **une seule variable globale, contenant uniquement un numéro de signal.**
Donc `env` et `exit_code` doivent être passés en paramètre. `t_shell` est créée une fois
dans `main` et son adresse circule.

```
main
 ├─ crée t_shell : copie de envp, exit_code = 0
 └─ boucle
      readline                     ← pas besoin de t_shell
      lexer                        ← pas besoin de t_shell
      parser                       ← pas besoin de t_shell
      expansion    (&shell)        ← env ($USER) et exit_code ($?)
      libère les tokens
      exécution    (&shell)        ← env pour execve, met à jour exit_code
      libère les commandes
```

C'est aussi la définition d'un builtin : une commande qui doit modifier `t_shell` elle-même.
`cd` ne peut pas être un programme externe — il changerait le répertoire de son fork, puis mourrait.

### Si tu remets les champs « confort »

Stocker `line`, `tokens`, `cmds` dans `t_shell` permet de tout libérer depuis n'importe où,
mais crée **deux propriétaires** pour la même mémoire (la variable locale de la boucle,
et le champ de `t_shell`).

**Règle non négociable : après chaque libération, remettre le champ à `NULL`.**
Sinon, double-free en fin de ligne.

### Pièges

- `env -i ./minishell` démarre avec un environnement **vide** : pas de `PATH`, pas de `PWD`.
  Ne pas segfault, et trouver quand même les builtins.
- `exit_code` doit être mis à jour après **chaque** commande : 127 introuvable, 126 permission,
  1 erreur de redirection, 2 erreur de syntaxe, 130 ctrl-C. **Un seul endroit qui écrit dedans.**
- Dans un pipe, `export` et `cd` ne modifient rien : le builtin tourne dans un fork, modifie
  la copie, et meurt. C'est le comportement attendu, pas un bug.

---

## 8. Exemple complet

```
echo a"b c" > out | wc -l
```

### Après le lexer — 7 tokens

```
 1                2                3                 4
┌───────────┐   ┌───────────┐   ┌─────────────┐   ┌───────────┐
│ MOT       │──►│ MOT       │──►│ REDIRECT    │──►│ MOT       │──┐
│ piece ●   │   │ piece ●   │   │ R_OUT       │   │ piece ●   │  │
└───────┬───┘   └───────┬───┘   │ piece NULL  │   └───────┬───┘  │
        │               │       └─────────────┘           │      │
        ▼               ▼                                 ▼      │
  ["echo",        ["a", Q_NONE]                     ["out",     │
   Q_NONE]           │next                           Q_NONE]    │
                     ▼                                           │
                  ["b c", Q_DOUBLE]                              │
                                                                 │
 ┌───────────────────────────────────────────────────────────────┘
 │   5              6                7
 │ ┌──────────┐   ┌───────────┐   ┌───────────┐
 └►│ PIPE     │──►│ MOT       │──►│ MOT       │──► NULL
   │piece NULL│   │ piece ●   │   │ piece ●   │
   └──────────┘   └───────┬───┘   └───────┬───┘
                          ▼               ▼
                     ["wc",Q_NONE]   ["-l",Q_NONE]
```

### Après le parser + expansion — 2 commandes

```
┌────────────────────────┐          ┌────────────────────────┐
│ t_command              │   next   │ t_command              │
│  argv    ●─────────────┼─────────►│  argv    ●             │──► NULL
│  redirs  ●             │          │  redirs  NULL          │
└────┬─────┬─────────────┘          └────┬───────────────────┘
     │     │                             │
     │     ▼                             ▼
     │  ┌────────────────┐        [ "wc" ][ "-l" ][ NULL ]
     │  │ type:  R_OUT   │
     │  │ target: "out"  │
     │  │ flag_quote: 0  │
     │  │ next:   NULL   │
     │  └────────────────┘
     ▼
 [ "echo" ][ "ab c" ][ NULL ]
                ▲
   les 2 morceaux concaténés, quotes retirées → UN seul argument
```

Trois choses à remarquer :

- **`out` n'est plus un token.** Il est devenu le `target` de la redirection ; il ne reste
  rien de lui dans `argv`.
- **Le `|` a disparu.** Ce n'était qu'une instruction de découpage, comme les quotes.
- **`a` + `b c` → `ab c`**, un seul argument contenant un espace.

---

## 9. Ordre de libération

**Toujours du plus interne vers le plus externe.**

```
libérer le contenu (argv, redirs, …)
puis  free(le maillon)
```

L'inverse est un *use-after-free* : `free()` rend la mémoire à l'allocateur, lire un champ
après coup déréférence une zone qui ne t'appartient plus. Ça **marche souvent** — l'allocateur
n'efface pas le contenu — jusqu'au jour où la zone est recyclée et où tu segfaultes ailleurs.
Valgrind, lui, le voit tout de suite : `Invalid read of size 8`.

Trois précisions :

- **Sauvegarder `next` avant** de libérer le maillon courant, sinon on relit un champ libéré.
- **`argv` a deux niveaux** : libérer chaque chaîne, **puis** le tableau.
- **`free(NULL)` est légal et sans effet.** Pas besoin de tester avant.

---
---

# Partie II — Les concepts shell

## 10. L'expansion

**Remplacer quelque chose par sa valeur, avant de lancer la commande.**

```
$ echo $USER
nico
```

`echo` ne reçoit **jamais** le texte `$USER`. Le shell fait le remplacement d'abord :

```
tu tapes :         echo $USER
le shell expanse : echo nico
echo reçoit :      argv = ["echo", "nico", NULL]
```

Trois expansions dans le sujet :

| Écrit | Remplacé par |
|---|---|
| `$USER` | la valeur de la variable dans la table d'env |
| `$?` | le code de sortie de la dernière commande |
| `$NEXISTEPAS` | **rien** (chaîne vide, pas d'erreur) |

Interaction avec les quotes :

```
$ echo "$USER"    →  nico     (double quote : on expanse)
$ echo '$USER'    →  $USER    (simple quote : littéral)
```

Et le cas qui justifie toute l'architecture, avec `A="a b"` :

```
$ echo $A         →  2 arguments : "a" et "b"
$ echo "$A"       →  1 argument  : "a b"
```

Sans le champ `quote` sur chaque `t_piece`, cette différence est impossible à produire.

> ⚠ L'expansion **duplique** la valeur trouvée dans `t_env`. Elle ne met jamais le pointeur
> de l'env dans `argv` — sinon un `unset` libérerait une chaîne encore utilisée.

---

## 11. Les redirections

### Les trois canaux

Tout processus a trois tuyaux ouverts, numérotés :

```
     ┌──────────────┐
     │              │──► 1    stdout   (ce que la commande affiche)
 0 ──►│  commande    │
     │              │──► 2    stderr   (les messages d'erreur)
     └──────────────┘
  ▲
  0   stdin  (ce que la commande lit)
```

Par défaut : `0` = le clavier, `1` et `2` = l'écran.

**Une redirection, c'est débrancher un de ces tuyaux et le rebrancher ailleurs.**

### Le sens du chevron = le sens des données

```
commande > fichier      les données SORTENT vers le fichier      (écriture)
commande < fichier      les données ENTRENT depuis le fichier    (lecture)
```

| | Sens | Ce qui suit | Effet |
|---|---|---|---|
| `>` | sortie | nom de fichier | crée / **vide** le fichier, puis écrit |
| `>>` | sortie | nom de fichier | crée si absent, écrit **à la fin** |
| `<` | entrée | nom de fichier | la commande lit le fichier au lieu du clavier |
| `<<` | entrée | **délimiteur** | la commande lit ce que tu tapes |

### Exemples

```
$ echo bonjour > f          # rien à l'écran, c'est parti dans f
$ cat f
bonjour

$ echo aaa > f              # > ÉCRASE
$ echo bbb > f
$ cat f
bbb

$ echo aaa >> g             # >> AJOUTE
$ echo bbb >> g
$ cat g
aaa
bbb
```

### `< fichier` vs `fichier` en argument

```
$ wc -l g       →  2 g      wc reçoit "g" dans argv, il ouvre le fichier lui-même
$ wc -l < g     →  2        wc ne reçoit AUCUN argument, le shell lui a branché g sur stdin
```

Dans le second cas, `g` **n'est pas dans `argv`** — c'est le `target` d'un `t_redirect`.
Voir §8.

---

## 12. Le heredoc `<<`

**« Here document » = un document écrit *ici*, dans la ligne de commande, au lieu d'être
dans un fichier.**

```
$ cat << FIN
> bonjour
> comment ça va
> FIN
bonjour
comment ça va
$
```

Déroulé :

1. Tu tapes `cat << FIN` et Entrée
2. Le shell affiche `>` et **attend que tu tapes des lignes**
3. Dès que tu tapes une ligne qui vaut **exactement** `FIN`, il s'arrête
4. Tout ce qui précède est envoyé sur le **stdin** de `cat`
5. `cat` l'affiche

`FIN` est le **délimiteur** : c'est le `target` du `t_redirect` quand `type == R_HEREDOC`.

### ⚠ `EOF` n'est pas un mot-clé

```
$ cat << EOF
$ cat << FIN
$ cat << STOP
$ cat << bonjour42
```

**Ces quatre lignes marchent identiquement.** Le délimiteur est un mot quelconque que *tu*
choisis. `EOF` est juste le plus utilisé par habitude (*End Of File*), il n'a **aucun statut
spécial**. Choisis un mot qui n'apparaît pas dans ton texte.

### ⚠ `>>` n'a aucun délimiteur

```
>> out          "out" est un NOM DE FICHIER
<< EOF          "EOF" est un DÉLIMITEUR
```

Les deux se ressemblent (deux chevrons) mais n'ont rien en commun.

### Toutes les commandes ne lisent pas stdin

| Lisent stdin | Ne lisent pas stdin |
|---|---|
| `cat`, `wc`, `grep`, `sort`, `head` | `echo`, `pwd`, `ls` |

```
$ echo bonjour < nimportequoi.txt
bonjour                            ← le fichier est ignoré
```

Donc brancher un heredoc sur `echo` ne sert à rien : tu peux taper 50 lignes, il ne les
verra jamais.

### Écrire un heredoc dans un fichier

```
$ cat << BENOIT > test
> bonjour
> deuxième ligne
> BENOIT
$ cat test
bonjour
deuxième ligne
```

```
ce que tu tapes  ──►  stdin de cat  ──►  cat recopie sur stdout  ──►  fichier test
   (heredoc <<)                                                          (> test)
```

`cat` sert de tuyau : il lit son entrée et la recrache sur sa sortie. C'est pour ça qu'on
l'utilise systématiquement avec les heredocs.

### Cas d'étude : `echo << BENOIT < test`

Syntaxiquement **valide** — le parser doit l'accepter. Ce qu'il produit :

```
argv    = ["echo", NULL]
redirs  = [ (R_HEREDOC, "BENOIT") ] ──► [ (R_IN, "test") ]
```

1. Le shell collecte le heredoc : il fait taper des lignes jusqu'à `BENOIT`
2. Applique la 1ʳᵉ redirection : stdin = le heredoc
3. Applique la 2ᵉ : stdin = `test` — **la première est écrasée** (liste ordonnée)
4. `echo` s'exécute, ignore stdin, affiche une ligne vide

Et si `test` n'existe pas, erreur à l'étape 3 et `echo` ne tourne pas.

> **Point d'implémentation :** le heredoc est collecté **avant l'exécution**, même s'il finit
> écrasé et même si la commande ne lira jamais rien. Les heredocs se lisent après le parsing,
> avant l'exécution, dans l'ordre, pour toute la ligne. Sinon `cat << A << B` est impossible.

Et ça rejoint `MINISHELL_GUIDE.md` §9 : le parser voit la **forme**, pas le sens.
Que la commande soit inutile n'est pas son problème.

---

## 13. Signaux et fin de saisie

### ctrl-D n'est pas un signal

| Touche | Nature | Effet |
|---|---|---|
| **ctrl-C** | signal `SIGINT` | interrompt ce qui tourne |
| **ctrl-\\** | signal `SIGQUIT` | quitte brutalement |
| **ctrl-D** | **pas un signal** | ferme l'entrée (EOF) |

Conséquence : **on ne peut pas attraper ctrl-D avec un gestionnaire de signal.** Il n'y en a
pas. Chercher à en poser un est une erreur classique.

`readline()` le donne gratuitement :

```
line != NULL   →  l'utilisateur a tapé une ligne
line == NULL   →  ctrl-D : plus d'entrée, on quitte le shell
```

Bash affiche `exit` avant de partir — ça se voit en soutenance.

### ctrl-D selon l'endroit

| Où | Comportement attendu |
|---|---|
| Prompt, **ligne vide** | le shell quitte (comme `exit`) |
| Prompt, **ligne commencée** | rien du tout — bash l'ignore |
| Pendant un **heredoc** | le heredoc se termine, avec un warning, et la commande **s'exécute** |

```
$ cat << FIN
> bonjour
> ^D
bash: warning: here-document at line 1 delimited by end-of-file (wanted `FIN')
bonjour              ← cat s'exécute quand même
```

### ctrl-C selon l'endroit

| Où | Comportement attendu |
|---|---|
| Prompt | jeter la ligne, nouveau prompt **sur une nouvelle ligne**, `$?` = 130 |
| Pendant un **heredoc** | abandonner le heredoc **et** la commande, `$?` = 130 |
| Pendant une commande (`sleep 10`) | tuer l'enfant, **pas** le shell, `$?` = 130 |

```
$ cat << FIN
> bonjour
> ^C
$ echo $?
130
```

**Dans les trois cas, le shell ne doit jamais mourir.**

### ctrl-C vs ctrl-D dans un heredoc — opposés

| | Heredoc | Commande |
|---|---|---|
| **ctrl-C** | abandonné | **pas exécutée**, `$?` = 130 |
| **ctrl-D** | terminé avec un warning | **exécutée** avec ce qui a été tapé |

### Les codes de sortie des signaux

`130` n'est pas magique : c'est **128 + le numéro du signal**.

| Signal | Numéro | Code |
|---|---|---|
| `SIGINT` (ctrl-C) | 2 | 130 |
| `SIGQUIT` (ctrl-\\) | 3 | 131 |

### Implications pour le code

- **C'est ici que sert la variable globale.** Une seule, contenant uniquement le numéro du
  signal — la seule autorisée par le sujet, et c'est son usage prévu.
- **Le heredoc partiel doit être libéré** quand ctrl-C arrive : lignes accumulées, descripteurs
  ouverts, `t_command` peut-être construite à moitié. D'où l'insistance sur les fonctions de
  libération **avant** d'en avoir besoin.

### Deux approches pour un heredoc interruptible

**A — un handler différent pendant le heredoc.** On change le comportement du signal avant la
saisie, on le remet après. Le handler positionne la globale, la boucle de lecture la teste.
Léger, mais `readline` est bloquant et l'interrompre proprement demande de la précision.

**B — forker pour collecter le heredoc.** L'enfant lit les lignes et les écrit dans un tuyau ;
dans l'enfant, ctrl-C le tue simplement. Le parent regarde **comment** l'enfant est mort :
tué par SIGINT → on abandonne la ligne, `$?` = 130. Plus de code, beaucoup plus robuste,
et gère naturellement `cat << A << B`.

**B est recommandée.** Le raisonnement « je regarde comment mon enfant est mort » est de toute
façon celui qu'il faudra écrire pour les commandes normales.

---
---

# Partie III — Contrat de propriété

> Livrable de Phase 0 (`MINISHELL_GUIDE.md` §7).
> *« Pour chaque struct définie, écrire sa fonction de libération dans la même heure. »*
>
> Rempli le 17 août 2026. Chaque ligne décrit ce que le code fait **aujourd'hui** — si le
> code change, cette page change avec lui.

| Struct | Qui l'alloue | Qui la libère | Quand exactement |
|---|---|---|---|
| `t_piece` | `piece_new`, via `read_piece` | `free_pieces`, appelée par `free_tokens` | avec son token propriétaire |
| `t_token` | `token_new`, via `read_word` / `read_op` | `free_tokens` | fin de `process_line`, après le parser |
| `t_redirect` | `redirect_new`, via `add_redirect` | `free_redirects`, appelée par `free_commands` | avec sa commande propriétaire |
| `t_command` | `command_new`, via `parser` | `free_commands` | fin de `process_line` ⚠ à revoir avec l'exécution |
| `t_env` | `env_new`, via `add_env_var` | `free_env` | `main`, après la boucle — vit toute la session |
| `t_shell` | pile de `main` | personne | libérée automatiquement au `return` |

### Décisions déjà prises

- [x] **Le parser copie** le `content` des `t_piece` pour construire `argv`. Il ne réutilise
      jamais le pointeur. Raisons : les durées de vie divergent ; un mot à plusieurs morceaux
      doit de toute façon être concaténé ; l'expansion alloue déjà.
      → **Chaque struct est propriétaire exclusive de ses chaînes.**

### Questions tranchées

- [x] **Qui libère la chaîne rendue par `readline()` ?**
      `main`, et personne d'autre — un `free(line)` à chaque tour de boucle. `lexer` ne fait
      que la lire caractère par caractère, il n'en garde aucun pointeur : tout ce qui entre
      dans un `t_piece` est une copie faite par `ft_substr`.

- [x] **Si une fonction échoue à mi-chemin, qui libère ce qui a déjà été construit ?**
      Celui qui construit, avant de renvoyer l'erreur. `lexer.c:32` appelle `free_tokens`
      avant de remonter son code ; `parser` appelle `free_commands`.
      → **Corollaire aussi important que la règle : l'appelant ne doit surtout pas
      re-libérer.** C'est pourquoi `process_line` ne libère rien après un `ERR_MALLOC` du
      lexer — il obtiendrait un double free.

- [ ] 🔴 **Où sont stockés les heredocs entre la collecte et l'exécution ?**
      Fichier temporaire / tuyau / chaîne en mémoire. **Seule question encore ouverte.**
      À décider avec le binôme : c'est lui qui branche le descripteur sur l'entrée standard.

- [x] **`t_shell` sur la pile ou sur le tas ?**
      Sur la pile de `main`, passée par adresse (`&shell`) à tout ce qui en a besoin. Une
      seule instance, durée de vie du programme, aucune allocation à gérer.

### Trois règles qui découlent de tout ça

**a. Un objet rejoint sa structure propriétaire dès sa création**, avant même d'être rempli.
`add_redirect` accroche le `t_redirect` à `cmd->redirs` *puis* remplit `target`. Si le
remplissage échoue, il n'y a rien à libérer localement — l'objet est déjà sous la
responsabilité de `free_commands`. Libérer soi-même à cet endroit produit un **double free**,
parce que `free_redirects` et `free_argv` prennent un simple pointeur et ne peuvent pas
remettre le champ du propriétaire à `NULL`.

**b. Les constructeurs adoptent, ils ne dupliquent pas.**
`env_new(key, value, exported)` range les deux pointeurs tels quels : le maillon en devient
propriétaire, `free_env` les libérera. Contrepartie à respecter par tout appelant : **si
`env_new` renvoie `NULL`, elle n'a rien adopté** — c'est à l'appelant de libérer `key` et
`value` avant de remonter l'erreur (`add_env_var` le fait).

**c. Toute fonction de libération d'une liste prend un double pointeur et remet
`*head = NULL`.** `free_tokens`, `free_commands`, `free_env`. C'est ce qui rend un second
appel inoffensif — et il y en a un : `init_env` nettoie en cas d'échec, puis `main` appelle
`free_env` à la sortie.

### Chaînes partagées entre les deux moitiés du projet

`t_env` est la seule structure écrite par l'un et lue par l'autre : les builtins `export` /
`unset` la modifient, l'expansion la lit.

> ⚠ Proposition à valider ensemble — `export` n'existe pas encore, cette règle est écrite
> en anticipation et non observée dans le code.

Règle proposée : **les builtins libèrent l'ancienne valeur avant d'en ranger une nouvelle**,
et n'insèrent que des chaînes fraîchement allouées — jamais un pointeur venu d'`argv`, qui
meurt avec la commande.

### Les cinq fonctions de libération

- [x] `t_piece` → `free_pieces` (`utils_list.c`)
- [x] `t_token` → `free_tokens`, descend dans les pieces (`utils_list.c`)
- [x] `t_redirect` → `free_redirects` (`utils_list2.c`)
- [x] `t_command` → `free_commands`, descend dans `argv` et `redirs` (`utils_list2.c`)
- [x] `t_env` → `free_env` (`utils_env.c`)
