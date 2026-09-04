/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_parser.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 19:55:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/15 19:55:01 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

char	*get_redir_symbol(t_type_redirect redir_type)
{
	if (redir_type == R_IN)
		return ("<");
	else if (redir_type == R_OUT)
		return (">");
	else if (redir_type == R_HEREDOC)
		return ("<<");
	else if (redir_type == R_APPEND)
		return (">>");
	return (NULL);
}

char	*token_symbol(t_token *token)
{
	t_type			type;
	t_type_redirect	redir_type;

	type = token->type;
	redir_type = token->redir_type;
	if (type == REDIRECT)
		return (get_redir_symbol(redir_type));
	else if (type == PIPE)
		return ("|");
	else if (type == UNEXPECTED_D)
		return ("&&");
	else if (type == UNEXPECTED_S)
		return ("&");
	else if (type == UNEXPECTED_P)
		return (";");
	return (NULL);
}

int	syntax_error(char *symbol)
{
	ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
	ft_putstr_fd(symbol, 2);
	ft_putstr_fd("'\n", 2);
	return (1);
}

int	check_syntax(t_token *tokens)
{
	t_token	*curr;

	if (tokens == NULL)
		return (0);
	if (tokens->type == PIPE)
		return (syntax_error(token_symbol(tokens)));
	curr = tokens;
	while (curr != NULL)
	{
		if (curr->type == UNEXPECTED_D || curr->type == UNEXPECTED_S
			|| curr->type == UNEXPECTED_P)
			return (syntax_error(token_symbol(curr)));
		else if (curr->type == PIPE && curr->next == NULL)
			return (syntax_error(token_symbol(curr)));
		else if (curr->type == PIPE && curr->next->type == PIPE)
			return (syntax_error(token_symbol(curr)));
		else if (curr->type == REDIRECT && curr->next == NULL)
			return (syntax_error("newline"));
		else if (curr->type == REDIRECT && curr->next->type != MOT)
			return (syntax_error(token_symbol(curr->next)));
		curr = curr->next;
	}
	return (0);
}
