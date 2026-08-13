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

#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum e_type
{
	MOT,
	PIPE,
    REDIRECT,
	UNEXPECTED_S,
	UNEXPECTED_D
}t_type;

typedef enum e_type_redirect
{
	R_NONE,
    R_IN,
	R_OUT,
	R_HEREDOC,
	R_APPEND
}t_type_redirect;

typedef enum e_quote
{
	Q_NONE,
    Q_SINGLE,
    Q_DOUBLE	
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
char	*ft_substr(char const *s, unsigned int start, size_t len);
t_token *token_new(t_type type);
t_piece *piece_new();
void	token_add_back(t_token **head, t_token *token);
void	piece_add_back(t_piece **head, t_piece *piece);
void	free_pieces(t_piece *head);
void	free_tokens(t_token **head);

#endif
