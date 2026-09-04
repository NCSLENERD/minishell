# Minishell — reste à faire

État au 2026-09-04, après la passe norminette.

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

## 1. Norme — ✅ 0 erreur

`norminette exec/ src/ *.c minishell.h` → **38 fichiers OK**, `norminette libft/` → **0 erreur**.

Seules restent deux `Notice: GLOBAL_VAR_DETECTED` (`minishell.h` et `src/utils/signal.c`) :
c'est `g_signal`, la variable globale unique autorisée par le sujet.

Fait dans cette passe :
- indentation entièrement ré-générée en tabulations d'après la profondeur d'accolades
- en-têtes 42 ajoutés aux 11 fichiers qui n'en avaient pas
- `}t_quote;` → `}\tt_quote;` et commentaires de fin de ligne retirés des enums —
  c'est `UNEXPECTED_D// &&` qui faisait **planter** norminette sur `minishell.h:130`,
  ce qui masquait toutes les erreurs du header
- les 126 prototypes du header alignés sur la même colonne
- `remove_env_key` et `get_backslash_char` raccourcies sous les 25 lignes
- `debug.c` supprimé (code mort, aucun appel hors du fichier) + retiré du Makefile

**Régler l'éditeur pour insérer des tabulations dans les `.c`**, sinon le problème revient.

## 2. Ménage avant rendu

- [x] Retirer `debug.c` et ses `print_debugger` / `print_token` / `print_commands` / `print_pieces_of_token`
- [ ] Supprimer les dossiers parasites : `ShellPy-main/`, `CLD/`, `l/`, `libft/a.out`
- [x] `.gitignore` en place (`*.out`, `*.o`, `*.a`, `minishell`)
- [x] Suppression de `src/minilibft/` bien commitée
- [ ] `libft/` : `ft_split.c` et `ft_memset.c` ont des `main()` commentés — les laisser commentés ou les supprimer

---

## 3. Robustesse

- [x] **Valgrind sous WSL** : 0 definitely / indirectly / possibly lost, 0 erreur,
      sur un scénario pipes + heredoc + redirections + builtins + `exit`.
- [x] **Signaux** testés dans un vrai pty :
      - Ctrl-C au prompt → nouvelle ligne + prompt ✅, mais **`$?` reste à 0 au lieu de 130** ❌
      - Ctrl-C pendant `sleep` → 130 ✅ ; pendant `cat` → le shell survit ✅
      - Ctrl-D sur ligne vide → `exit` et quitte ✅
      - Ctrl-C dans un heredoc → heredoc annulé, shell vivant ✅, mais `$?` = 0 au lieu de 130 ❌
      - Ctrl-\\ au prompt → rien ✅ ; pendant `sleep` → `Quit` + 131 ✅
- [x] **`env -i ./minishell`** : sans aucun environnement, `export` / `pwd` / `cd` / `echo $PATH` doivent tenir sans segfault

---

## 4. Bugs restants

1. **`$?` = 0 au lieu de 130 après un Ctrl-C au prompt** (et dans un heredoc).
   `src/utils/signal.c` pose `SA_RESTART` sur SIGINT, donc `readline()` ne rend jamais
   la main : le test `if (g_signal != 0)` de `main.c` ne s'exécute qu'après la commande
   *suivante*, dont le code retour l'écrase aussitôt.

2. **`echo "\$USER"`** sort `\rlaghsal` au lieu de `$USER`. Le backslash est hors sujet,
   mais il est implémenté à moitié : dans des guillemets doubles il devrait échapper le `$`.

3. Cosmétique : `exit` dans un pipe affiche `exit` (pas bash) ; `unset PATH ; ls` dit
   `command not found` là où bash dit `No such file or directory`.

## 5. Ménage dépôt

- **51 fichiers de `CLD/` sont commités** (dont des `.md` et un PDF) — à retirer.
