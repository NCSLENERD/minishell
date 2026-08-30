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

int	is_dollar_open(int i, char *line)	
{
	if (line[i] == '$' && get_quote_type(line, i + 1) != Q_NONE)
		return (1);
	return (0);
}

int	read_piece2(char *line, int *i, int	*start, t_quote *quote)
{
	int	flag_backslash;

	flag_backslash = 0;
	if (is_dollar_open(*i, line) && line[*i + 1] == '\'')
	{
		*i = *i + 1;
		flag_backslash = 1;
	}
	else if(is_dollar_open(*i, line))
		*i = *i + 1;	
	*quote = get_quote_type(line, *i);
	if (*quote != Q_NONE)
		*i = *i +1;
	*start = *i;
	while (!is_end_of_piece(line[*i], *quote, i, line) && line[*i] != '\0')
	{
		if (line[*i] == '\\' && flag_backslash == 1 && line[*i + 1] != '\0')
			*i = *i + 1;
		*i = *i +1;
	}
	return (flag_backslash);
}

int	read_piece3(t_piece *piece, int flag_backslash)
{
	char	*stock;

	if (flag_backslash == 1)
	{
		stock = trad_backslash(piece->content);
		if(!stock)
			return (ERR_MALLOC);
		free(piece->content);
		piece->content = stock;
	}
	return (0);
}

int	read_piece(char *line, int *i, t_token *token)
{
	int	start;
	int	flag_backslash;
	t_piece *piece;
	t_quote quote;

	flag_backslash = read_piece2(line, i, &start, &quote);
	if (line[*i] == '\0' && quote != Q_NONE)
	{
		quote_error(get_quote_symbol(quote));
		return (ERR_SYNTAX); // erreur car quote non fermer
	}
	piece = piece_new(quote);
	if (!piece)
		return (ERR_MALLOC);
	piece_add_back(&token->piece, piece);
	piece->content = ft_substr(line, start, (*i - start));
	if (!piece->content)
		return (ERR_MALLOC);
	if (read_piece3(piece, flag_backslash) == ERR_MALLOC)
		return (ERR_MALLOC);
	if (quote != Q_NONE)
		*i = *i + 1;
	return (0);
}
