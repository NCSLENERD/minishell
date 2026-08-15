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

t_token *token_new(t_type type)
{
	t_token *new;

	new = malloc(sizeof(t_token));
	if (!new)
		return (NULL);
	new->type = type;
	new->redir_type = R_NONE;
	new->piece = NULL;
	new->next = NULL;
	return (new);
}

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

void	token_add_back(t_token **head, t_token *token)
{
	t_token	*curr;
	
	if(*head == NULL)
	{
		*head = token;
		return ;
	}
	curr = *head;
	while (curr != NULL)
	{
		if (curr->next == NULL)
		{
			curr->next = token;
			break;
		}
		curr = curr->next;
	}
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

void	free_tokens(t_token **head)
{
	t_token	*curr1;
	t_token	*curr2;
	
	if(*head == NULL)
		return ;
	
	curr1 = *head;
	curr2 = NULL; 
	while (curr1 != NULL)
	{
		curr2 = curr1->next;
		free_pieces(curr1->piece);
		free(curr1);
		curr1 = curr2;
	}
	*head = NULL;
}
