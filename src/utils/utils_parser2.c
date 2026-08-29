/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_parser2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 15:12:46 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/16 15:12:47 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

int	calc_len_piece(t_piece *piece)
{
	int	len;
	t_piece	*curr;

	curr = piece;
	len = 0;
	while (curr != NULL)
	{
		len += ft_strlen(curr->content);
		curr = curr->next;
	}
	return (len);
}

char	*piece_to_str(t_piece	*piece)
{
	int	len;
	char	*str;

	len = calc_len_piece(piece);
	str = malloc(sizeof(char) * (len + 1));
	if (!str)
		return (NULL);
	piece_to_str2(piece, str);
	return (str);
}

void	piece_to_str2(t_piece *piece,  char *str)
{
	int	i;
	int	j;
	t_piece *curr;

	i = 0;
	j = 0;
	curr = piece;
	while (curr != NULL)
	{
		while (curr->content[j] != '\0')
		{
			str[i] = curr->content[j];
			i++;
			j++;
		}
		j = 0;
		curr = curr->next;
	}
	str[i] = '\0';
}