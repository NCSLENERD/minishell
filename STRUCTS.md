# Minishell — Référence des structures

> Ce document décrit **la forme** de chaque struct : ses champs, quand ils sont valides,
> et à quoi ressemblent les données en mémoire.
>
> Il ne contient **pas** le contrat de propriété (qui alloue / qui libère / durée de vie) :
> c'est la section 8, laissée à remplir. Voir `MINISHELL_GUIDE.md` §7.

---

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
char        *line;          /* confort : libération en cas d'erreur       */
t_token     *tokens;        /* confort : idem                             */
t_command   *cmds;          /* confort : idem                             */
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

### ⚠ Les champs « confort »

Ils permettent de tout libérer depuis n'importe où, mais créent **deux propriétaires** pour la
même mémoire (la variable locale de la boucle, et le champ de `t_shell`).

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

## 10. Contrat de propriété — À REMPLIR

> Livrable de Phase 0 (`MINISHELL_GUIDE.md` §7).
> *« Pour chaque struct définie, écrire sa fonction de libération dans la même heure. »*

| Struct | Qui l'alloue | Qui la libère | Quand exactement |
|---|---|---|---|
| `t_piece` | | | |
| `t_token` | | | |
| `t_redirect` | | | |
| `t_command` | | | |
| `t_env` | | | |
| `t_shell` | | | |

### Questions à trancher avant d'écrire le lexer

- [ ] Quand le parser construit `argv`, il **copie** le `content` des `t_piece` ou il **réutilise** le pointeur ?
- [ ] Qui libère la chaîne rendue par `readline()` ?
- [ ] Si le parser échoue à mi-chemin (`ls |`, `> ` en fin de ligne), qui libère l'arbre à moitié construit ?
- [ ] Quand l'expansion remplace `$USER`, elle duplique la valeur de `t_env` ou elle pointe dessus ?
- [ ] Après avoir libéré `shell->tokens`, qui remet le champ à `NULL` ?

### Les cinq fonctions de libération à écrire

- [ ] `t_piece`
- [ ] `t_token` (descend dans les pieces)
- [ ] `t_redirect`
- [ ] `t_command` (descend dans argv et redirs)
- [ ] `t_env`
