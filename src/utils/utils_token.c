/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_token.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 12:00:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/09/04 12:00:00 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

t_token	*token_new(t_type type)
{
	t_token	*new;

	new = malloc(sizeof(t_token));
	if (!new)
		return (NULL);
	new->type = type;
	new->redir_type = R_NONE;
	new->piece = NULL;
	new->next = NULL;
	return (new);
}

void	token_add_back(t_token **head, t_token *token)
{
	t_token	*curr;

	if (*head == NULL)
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
			break ;
		}
		curr = curr->next;
	}
}

void	free_tokens(t_token **head)
{
	t_token	*curr1;
	t_token	*curr2;

	if (*head == NULL)
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
