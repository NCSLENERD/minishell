/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_list2.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 19:16:17 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/15 19:16:18 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

t_redirect *redirect_new(t_type_redirect type)
{
	t_redirect *new;

	new = malloc(sizeof(t_redirect));
	if (!new)
		return (NULL);
	new->type = type;
	new->target = NULL;
	new->flag_quote = 0;
	new->next = NULL;
	return (new);
}

void	redirect_add_back(t_redirect **head, t_redirect *redirect)
{
	t_redirect	*curr;
	
	if(*head == NULL)
	{
		*head = redirect;
		return ;
	}
	curr = *head;
	while (curr != NULL)
	{
		if (curr->next == NULL)
		{
			curr->next = redirect;
			break;
		}
		curr = curr->next;
	}
}

void	free_redirects(t_redirect *head)
{
	t_redirect	*curr1;
	t_redirect	*curr2;
	
	curr1 = head;
	curr2 = NULL; 
	while (curr1 != NULL)
	{
		curr2 = curr1->next;
		free(curr1->target);
		free(curr1);
		curr1 = curr2;
	}
}