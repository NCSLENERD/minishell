/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_parser3.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 12:00:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/09/04 12:00:00 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

int	add_field(t_piece **fields, char *str)
{
	t_piece	*new;

	new = piece_new(Q_NONE);
	if (!new)
	{
		free(str);
		return (ERR_MALLOC);
	}
	new->content = str;
	piece_add_back(fields, new);
	return (0);
}

int	split_piece2(char *content, char **acc, int *started, int *i)
{
	int		start;
	char	*slice;

	start = *i;
	while (content[*i] && !is_space(content[*i]))
		*i = *i + 1;
	slice = ft_substr(content, start, *i - start);
	if (!slice)
	{
		free(*acc);
		*acc = NULL;
		return (ERR_MALLOC);
	}
	*acc = expand_append(*acc, slice);
	free(slice);
	if (*acc == NULL)
		return (ERR_MALLOC);
	*started = 1;
	return (0);
}

int	split_piece3(t_piece **fields, char **acc, int *started)
{
	if (*started == 1)
	{
		if (add_field(fields, *acc) == ERR_MALLOC)
		{
			*acc = NULL;
			return (ERR_MALLOC);
		}
		*acc = ft_strdup("");
		if (*acc == NULL)
			return (ERR_MALLOC);
		*started = 0;
	}
	return (0);
}

int	split_piece(char *content, t_piece **fields, char **acc, int *started)
{
	int	i;

	i = 0;
	while (content[i])
	{
		if (is_space(content[i]))
		{
			if (split_piece3(fields, acc, started) == ERR_MALLOC)
				return (ERR_MALLOC);
			i++;
		}
		else
		{
			if (split_piece2(content, acc, started, &i) == ERR_MALLOC)
				return (ERR_MALLOC);
		}
	}
	return (0);
}
