/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_lexer.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 14:34:26 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/15 14:34:27 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

t_type_redirect get_redir_type(char *line, int *i)
{
	if (line[*i] == '>' && line[*i + 1] == '>')
		return (R_APPEND);
	else if (line[*i] == '<' && line[*i + 1] == '<')
		return (R_HEREDOC);
	else if (line[*i] == '>')
		return (R_OUT);
	else if (line[*i] == '<')
		return (R_IN);
	return (R_NONE);	
}

int	read_op(char *line, int *i, t_token **head)
{
	t_token *token;
	t_type type;
	t_type_redirect redir_type;

	type = UNEXPECTED_S;
	if (line[*i] == '>' || line[*i] == '<')
		type = REDIRECT;
	else if (line[*i] == '|')
		type = PIPE;
	else if (line[*i] == '&' && line[*i + 1] == '&')
		type = UNEXPECTED_D;
	else if (line[*i] == '&')
		type = UNEXPECTED_S;
	else if (line[*i] == ';')
		type = UNEXPECTED_P;
	redir_type = get_redir_type(line, i);
	if (line[*i] == line[*i + 1] && type != PIPE)
		*i = *i + 1;
	*i = *i + 1;
	token = token_new(type);
	if (!token)
		return (ERR_MALLOC);
	token->redir_type = redir_type;
	token_add_back(head, token);
	return (0);
}

int	read_word(char *line, int *i, t_token **head)
{
	t_token	*token;
	int	ret;

	token = token_new(MOT);
	if (!token)
		return (ERR_MALLOC);
	token_add_back(head, token);
	while (line[*i] != '\0' && !is_operator(line[*i]) && !is_space(line[*i]))
	{
		ret = read_piece(line, i, token);
		if (ret != 0)
			return (ret);
	}
	return (0);
}

int	is_end_of_piece(char c, t_quote quote, int *i, char *line)
{
	if (c == '\'' && quote == Q_SINGLE)
		return (1);
	else if (c == '"' && quote == Q_DOUBLE)
		return (1);
	else if ((c == '\0' || c == '"' || c == '\'' || is_operator(c) || 
	is_space(c) || is_dollar_open(*i, line) ) 
	&& quote == Q_NONE)
		return (1);
	return (0);
}

