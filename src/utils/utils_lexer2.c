/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_lexer2.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 21:14:50 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/15 21:14:51 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

int	is_space(char c)
{
	if (c == ' ' || c == '\t')
		return (1);
	return (0);
}

int	is_operator(char c)
{
	if (c == '>' || c == '<' || c == '&' || c == ';' || c == '|')
		return (1);
	return (0);
}

t_quote get_quote_type(char *line, int i)
{
	if (line[i] == '"')
		return (Q_DOUBLE);
	else if (line[i] == '\'')
		return (Q_SINGLE);
	return (Q_NONE);	
}

char	*get_quote_symbol(t_quote quote)
{
	if (quote == Q_DOUBLE)
		return ("\"");
	else if (quote == Q_SINGLE)
		return ("'");
	return (NULL);
}

int	quote_error(char *symbol)
{
	ft_putstr_fd("minishell: unexpected EOF while looking for matching `", 2);
	ft_putstr_fd(symbol, 2);
	ft_putstr_fd("'\n", 2);
	return (1);
}