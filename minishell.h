/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 19:55:05 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/12 19:32:10 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef MINISHELL_H
# define MINISHELL_H
# define ERR_SYNTAX 1
# define ERR_MALLOC 2
# define ERR_REDIR 3

#include "libft/libft.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

typedef enum e_type
{
	MOT,
	PIPE,
    REDIRECT,
	UNEXPECTED_P,//;
	UNEXPECTED_S,//&
	UNEXPECTED_D//&&
}t_type;

typedef enum e_type_redirect
{
	R_NONE,
    R_IN, // <
	R_OUT, // >
	R_HEREDOC, // <<
	R_APPEND // >>
}t_type_redirect;

typedef enum e_quote
{
	Q_NONE,
    Q_SINGLE, // ''
    Q_DOUBLE // ""
}t_quote;

typedef struct s_piece
{
    char *content;
    t_quote quote;
    struct s_piece *next;
} t_piece;

typedef	struct s_redirect
{
	t_type_redirect    type;
	int	fd;
	char	*target;
	int		flag_quote;
	struct s_redirect *next;
} t_redirect;

typedef struct s_command
{
	char	**argv; 
	t_redirect	*redirs; // Si redirection <,>,<<,>> 
	struct s_command *next;
}t_command;

typedef struct s_token
{
    t_type type;
	t_type_redirect redir_type; // type de redirection
    t_piece *piece; // FORMAT   token = echo a "b c" 'd' piece = [[a ,NONE] [b c, DOUBLE] [d,SINGLE]] 
	struct s_token *next;
} t_token;
 
typedef struct s_env
{
	char	*key;
	char	*value;
	int	exported; // flag pour savoir si visible dans le env ou seulement dans le export ( export A (dans export) / export A=10 (dans env))
	struct s_env *next;
} t_env;

typedef struct s_shell
{
	t_env	*env;
	int	exit_code;
	//char *line; // la ligne actuelle de readline ,CONFORT pour free en cas d'erreur
	//t_token *tokens;// CONFORT pour free en cas d'erreur 
	//t_command *cmds;// CONFORT pour free en cas d'erreur 
} t_shell;

t_token *token_new(t_type type);
t_piece *piece_new(t_quote quote);
void	token_add_back(t_token **head, t_token *token);
void	piece_add_back(t_piece **head, t_piece *piece);
void	free_pieces(t_piece *head);
void	free_tokens(t_token **head);
int	lexer(char *line, t_token **head);
void	print_token(t_token *token); // temporaire
int	is_operator(char c);
int	is_space(char c);
int	read_op(char *line, int *i, t_token **head);
int	read_word(char *line, int *i, t_token **head);
int	read_piece(char *line, int *i, t_token *token);
int	is_end_of_piece(char c, t_quote quote, int *i, char *line);
int	is_dollar_open(int i, char *line);
char	*trad_backslash(char *content);
void	trad_backslash2(char *content, char *res, int *i, int *j);
char	get_backslash_char(char c);
t_quote	get_quote_type(char *line, int i);
int	quote_error(char *symbol);
char	*get_quote_symbol(t_quote quote);
void	free_commands(t_command **head);
void	free_redirects(t_redirect *head);
void	command_add_back(t_command **head, t_command *command);
void	redirect_add_back(t_redirect **head, t_redirect *redirect);
t_redirect *redirect_new(t_type_redirect type);
t_command *command_new();
int	is_expandable(char *content, int i);
char	*expand_append(char *s1, char *s2);
char	*expand_str(char *content, t_shell *shell);
char	*expand_code(t_shell *shell, int *i, char *acc);
char	*expand_var(char *content, t_shell *shell, int *i, char *acc);
char	*expand_litteral(char *content, int *i, char *acc);
int	expand_tokens(t_token *tokens, t_shell *shell);
int	expand_piece(t_piece *piece, t_shell *shell);
void	free_argv(char **argv);
int	check_syntax(t_token *tokens);
char	*get_redir_symbol(t_type_redirect redir_type);
int	calc_len_piece(t_piece *piece);
char	*piece_to_str(t_piece	*piece);
void	piece_to_str2(t_piece *piece, char *str);
void	print_pieces_of_token(t_token *token); 
int parser(t_token *tokens, t_command **head);
int	handle_heredoc(t_redirect	*new, t_token *token);
int	add_field(t_piece **fields, char *str);
int	split_piece(char *content, t_piece **fields, char **acc, int *started);
int	split_piece2(char *content, char **acc, int *started, int *i);
int	split_piece3(t_piece **fields, char **acc, int *started);
int	split_word(t_token *token, t_piece **fields);
int	count_fields(t_piece *fields);
int	fields_to_argv(t_command *cmd, t_piece *fields);
void	print_commands(t_command *commands);
int	fill_command(t_token **tokens, t_command *cmd);
void	print_debugger(t_token *tokens, t_command *commands);
int	init_env(char **envp, t_env **head);
int	add_env_var(char *str, t_env **head);
void	env_add_back(t_env **head, t_env *env);
t_env	*env_new(char *key, char *value, int flag_exported);
void	free_env(t_env **head);
int	execute(t_command *cmds, t_shell *shell);
void	exec_child(t_command *cmd, t_shell *shell);
void	set_exit_status(t_shell *shell, int status);
char	**env_to_tab(t_env *env);
int	count_env_var(t_env *env);
char	*make_env_entry(t_env *var);
int	fill_env_tab(char **tab, t_env *env);
char	*get_env_value(t_env *env, char *key);
char	*join_path(char *dir, char *cmd);
char	*search_in_path(char **dirs, char *cmd);
char	*find_path(char *cmd, t_env *env);
int	cmd_error(char *cmd, char *msg, int code);
int	execve_error(char *cmd);
int	is_directory(char *path);
int	open_redir(t_redirect *redir);
int	apply_redirs(t_redirect *redirs);
void	child_pipe_setup(int prev_fd, int *fds, t_command *cmd);
int	parent_pipe_setup(int prev_fd, int *fds, t_command *cmd);
void	wait_all(pid_t last, t_shell *shell);
pid_t	launch_command(t_command *cmd, t_shell *shell, int prev_fd, int *fds);
int	run_pipeline(t_command *cmds, t_shell *shell);
int	nothing_to_do(t_command *cmds);
int	is_builtin(char *name);
int	run_builtin(t_command *cmd, t_shell *shell);
int	run_builtin_parent(t_command *cmd, t_shell *shell);
int	is_n_flag(char *s);
int	builtin_echo(char **argv);
int	builtin_pwd(void);
int	builtin_env(t_env *env);
int	builtin_exit(t_command *cmd, t_shell *shell);
int	parse_exit_code(char *s, unsigned char *code);
int	parse_sign(char *s, int *i);
int	exit_error(char *arg);
void	clean_exit(t_shell *shell, unsigned char code);
int	collect_heredocs(t_command *cmds, t_shell *shell);
int	read_heredoc(t_redirect *redir, t_shell *shell);
int	is_delimiter(char *line, char *delim);
int	write_heredoc_line(int fd, char *line, t_redirect *redir, t_shell *shell);
void	close_heredocs(t_command *cmds);
void	heredoc_warning(char *delim);
char	*heredoc_path(void);
int	fill_heredoc(int fd, t_redirect *redir, t_shell *shell);
int	set_env_value(t_env **env, char *key, char *value);
int	add_new_env(t_env **env, char *key, char *value);
int	builtin_cd(t_command *cmd, t_shell *shell);
char	*get_cd_target(t_command *cmd, t_shell *shell);
int	update_pwd(t_shell *shell, char *oldpwd);
int	cd_error(char *arg, char *msg);
#endif
 