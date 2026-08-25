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

char	*ft_strdup(const char *s);
size_t	ft_strlen(const char *str);
char	*ft_itoa(int n);
char	*ft_substr(char const *s, unsigned int start, size_t len);
void	ft_putstr_fd(char *s, int fd);
char	*ft_strchr(const char *s, int c);
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
t_quote get_quote_type(char *line, int *i);
int	quote_error(char *symbol);
char	*get_quote_symbol(t_quote quote);
void	free_commands(t_command **head);
void	free_redirects(t_redirect *head);
void	command_add_back(t_command **head, t_command *command);
void	redirect_add_back(t_redirect **head, t_redirect *redirect);
t_redirect *redirect_new(t_type_redirect type);
t_command *command_new();
void	free_argv(char **argv);
int	check_syntax(t_token *tokens);
char	*get_redir_symbol(t_type_redirect redir_type);
int	calc_len_piece(t_piece *piece);
char	*ft_strjoin(char const *s1, char const *s2);
char	*piece_to_str(t_piece	*piece);
void	piece_to_str2(t_piece *piece, char *str);
void	print_pieces_of_token(t_token *token);
int parser(t_token *tokens, t_command **head);
void	print_commands(t_command *commands);
int	count_argv(t_token *tokens);
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
#endif
 