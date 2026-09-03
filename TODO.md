# Minishell — reste à faire

État au 2026-09-03, commit `8dacf58 unset done`.

**Le bonus n'est pas fait** : on s'en tient à l'obligatoire.

---

## ✅ Fait — obligatoire fonctionnellement complet

Testé contre bash, tout passe :

| Domaine | Détail |
|---|---|
| Builtins | `echo -n` (dont `-n -n -nnn`), `cd` / `cd -` / `cd` seul → `$HOME`, `pwd`, `export`, `unset`, `env`, `exit` |
| `exit` | `exit 999` → 231, `exit abc` → erreur + code 2, `exit 1 2` → erreur et ne quitte pas |
| Quotes | `'$USER'` littéral, `"$USER"` étendu, `a"b"c` → `abc` |
| Expansion | `$USER$USER`, `$INEXISTANT` → vide, `"$"` → `$`, `$?` |
| Redirections | `<` `>` `>>` `<<`, heredoc quoté (pas d'expansion), heredoc dans un pipe |
| Pipes | `ls \| head`, `cat \| cat \| cat` |
| Codes retour | 127 (command not found), 1 (erreur cmd), 2 (syntaxe) |
| Erreurs syntaxe | `\|` seul, `ls \|`, guillemet non fermé → message + code 2 |
| `unset PATH` puis `ls` | `command not found`, code 127 |
| Fuites | 0 leak sur le scénario export/unset (`leaks` macOS) |

---

## 1. Norme — 744 erreurs — BLOQUANT

Un seul fichier non conforme = 0 au projet, avant même que le shell soit lancé.

### 1a. Whitespace — 501 erreurs, 2 commandes

```
221  TOO_FEW_TAB
218  SPACE_REPLACE_TAB
 62  SPACE_EMPTY_LINE
```

```sh
perl -pi -e '1 while s/^(\t*) {4}/$1\t/' exec/*.c src/utils/*.c *.c minishell.h
perl -pi -e 's/[ \t]+$//'                exec/*.c src/utils/*.c *.c minishell.h
```

Puis **régler l'éditeur pour insérer des tabulations dans les `.c`**, sinon le problème revient.

### 1b. Fonctions > 25 lignes — vrai découpage

- [ ] `exec/builtin_unset.c` → `remove_env_key` : factoriser le `if/else` (les deux branches finissent par les mêmes lignes)
- [ ] `src/utils/utils_lexer4.c:42`
- [ ] `debug.c:48` — disparaît si `debug.c` est retiré (voir §2)

### 1c. Le reste — 243 erreurs à la main

En-têtes 42 manquants (11 fichiers), `if(`/`while(`/`return(` → espace avant la parenthèse,
`t_env   *curr;` → tabulation et non espaces, alignement des déclarations, retour à la ligne en fin de fichier.

### Pires fichiers

| fichier | erreurs |
|---|---|
| `exec/builtin_export2.c` | 176 |
| `exec/builtin_export.c` | 169 |
| `exec/builtin_unset.c` | 116 |
| `src/utils/signal.c` | 68 |
| `exec/builtin_env.c` | 22 |

Vérification : `norminette exec/ src/ *.c minishell.h`

---

## 2. Ménage avant rendu

- [ ] Retirer `debug.c` et ses `print_debugger` / `print_token` / `print_commands` / `print_pieces_of_token`
- [ ] Supprimer les dossiers parasites : `ShellPy-main/`, `CLD/`, `l/`, `libft/a.out`
- [ ] Ajouter un `.gitignore` : `*.o`, `*.a`, `minishell`, `a.out`
- [ ] Vérifier que la suppression de `src/minilibft/` est bien commitée
- [ ] `libft/` : `ft_split.c` et `ft_memset.c` ont des `main()` commentés — les laisser commentés ou les supprimer

---

## 3. Robustesse

- [ ] **Valgrind sous WSL** : `valgrind --leak-check=full --show-leak-kinds=all`
      sur des pipes, des heredocs, et un `exit` après plusieurs commandes.
      Le `leaks` macOS ne couvrait que les builtins.
- [ ] **Signaux en interactif** (impossible à tester en mode piped) :
      - Ctrl-C au prompt → nouvelle ligne + prompt, `$?` = 130
      - Ctrl-C pendant un `cat` → le shell survit
      - Ctrl-D sur ligne vide → quitte
      - Ctrl-D dans un heredoc → warning bash
      - Ctrl-\ au prompt → rien
- [ ] **`env -i ./minishell`** : sans aucun environnement, `export` / `pwd` / `cd` / `echo $PATH` doivent tenir sans segfault
