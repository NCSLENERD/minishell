*This project has been created as part of the 42 curriculum by nmayela, rlaghsal*

# minishell

## Description

`minishell` is a simplified command interpreter written in C, built as a
reimplementation of a subset of `bash`. It reads a line, splits it into tokens,
expands variables, parses the result into commands, and executes them.

It supports:

- **Builtins** — `echo` (with `-n`), `cd` (relative, absolute, `~`, `-`, no
  argument), `pwd`, `export`, `unset`, `env`, `exit`
- **Redirections** — `<`, `>`, `>>`, and heredoc `<<`; a quoted delimiter
  disables expansion, as in bash
- **Pipelines** — `cmd1 | cmd2 | cmd3`, with no limit on the number of stages
- **Quoting** — single quotes taken literally, double quotes with expansion,
  and concatenation (`a"b"c` yields `abc`)
- **Expansion** — `$VAR` and `$?`; an undefined variable expands to an empty
  string
- **Signals** — `Ctrl-C` prints a newline and a fresh prompt, `Ctrl-D` exits,
  `Ctrl-\` is ignored at the prompt
- **Exit statuses** — `0`, `1`, `2` for syntax errors, `127` for a command that
  was not found, `128+n` when a child is killed by signal `n`

A single global variable is used, as the subject requires: `g_signal`, of type
`volatile sig_atomic_t`, which only records the signal number received by the
handler.

Not supported, as they fall outside the mandatory part: `;`, `&&`, `||`,
wildcards, and subshells.

## Instructions

Requires **readline** (`libreadline-dev` on Debian and Ubuntu).

```sh
make        # builds ./minishell, and libft along the way
make clean  # removes the object files
make fclean # removes the object files and the binary
make re     # fclean followed by make
```

Then run it:

```sh
./minishell
minishell$ echo hello | tr a-z A-Z
HELLO
minishell$ exit
```

Source layout:

```
main.c  lexer.c  parser.c   main loop, tokenizing, parsing
minishell.h                 structures and prototypes
src/utils/                  lexer, parser, expansion, signals
exec/                       execution, pipes, redirections, builtins
libft/                      in-house C library
```

## Resources

- `man bash`, and the POSIX shell command language specification, used as the
  reference for behaviour we could not derive from testing alone
- The GNU readline documentation, for `readline`, `add_history`,
  `rl_replace_line`, `rl_on_new_line` and `rl_redisplay`
- The `man` pages for `fork`, `execve`, `pipe`, `dup2`, `waitpid`, `chdir`,
  `getcwd` and `sigaction`
- `bash` itself, used as an executable oracle: a test harness runs the same
  command through `bash` and through `minishell` and compares standard output,
  standard error and exit status
- `valgrind`, to confirm that nothing is leaked

## Use of AI

AI assistance (Claude, through Claude Code) was used on this project. What it
was used for, and what it was not:

**Where it was used**

- *Norm compliance.* The project had 775 norminette errors. AI wrote a
  reformatting script that regenerated indentation from brace depth while
  protecting string literals, and located the cause of a norminette crash on
  `minishell.h` that was hiding roughly seventy header errors.
- *Testing against bash.* AI wrote the differential test harness described
  above, which runs around 120 commands through both shells and compares the
  results, along with a pseudo-terminal harness used to test signal handling.
- *Memory analysis.* AI ran and interpreted valgrind, and attributed the
  remaining reachable blocks to readline rather than to our own code.

## Authors

Work was split two ways, with the main areas of responsibility being:

- **nmayela** — lexer, parser, expansion, signals (`src/utils/`)
- **rlaghsal** — execution, pipes, redirections, builtins (`exec/`)

Neither split was exclusive: both of us worked on both sides.
