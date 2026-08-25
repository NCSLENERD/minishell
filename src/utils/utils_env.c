/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_env.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 22:37:28 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/16 22:37:29 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

void	free_env(t_env **head)
{
	t_env	*curr1;
	t_env	*curr2;
	
	curr1 = *head;
	curr2 = NULL; 
	while (curr1 != NULL)
	{
		curr2 = curr1->next;
		free(curr1->key);
		free(curr1->value);
		free(curr1);
		curr1 = curr2;
	}
	*head = NULL;
}

t_env	*env_new(char *key, char *value, int flag_exported)
{
	t_env	*new;

	new = malloc(sizeof(t_env));
	if (!new)
		return (NULL);
	new->key = key;
	new->value = value;
	new->exported = flag_exported;
	new->next = NULL;
	return (new);
}

void	env_add_back(t_env **head, t_env *env)
{
	t_env *curr;
	
	if(*head == NULL)
	{
		*head = env;
		return;
	}
	curr = *head;
	while (curr != NULL)
	{
		if (curr->next == NULL)
		{
			curr->next = env;
			break;
		}
		curr = curr->next;
	}
}