/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 20:24:28 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/12 20:24:30 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

t_piece *piece_new(t_quote quote)
{
	t_piece *new;

	new = malloc(sizeof(t_piece));
	if (!new)
		return (NULL);
	new->content = NULL;
    new->quote = quote;
    new->next = NULL;
	return (new);
}

void	piece_add_back(t_piece **head, t_piece *piece)
{
	t_piece	*curr;
	
	if(*head == NULL)
	{
		*head = piece;
		return ;
	}
	curr = *head;
	while (curr != NULL)
	{
		if (curr->next == NULL)
		{
			curr->next = piece;
			break;
		}
		curr = curr->next;
	}
}

void	free_pieces(t_piece *head)
{
	t_piece	*curr1;
	t_piece	*curr2;
	
	curr1 = head;
	curr2 = NULL; 
	while (curr1 != NULL)
	{
		curr2 = curr1->next;
		free(curr1->content);
		free(curr1);
		curr1 = curr2;
	}
}