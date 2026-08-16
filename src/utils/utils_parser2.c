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
	int	i;
	int	j;
	int	len;
	t_piece	*curr;
	char	*str;

	curr = piece;
	i = 0;
	j = 0;
	len = calc_len_piece(piece);
	str = malloc(sizeof(char) * (len + 1));
	if (!str)
		return (NULL);
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
	return (str);
}