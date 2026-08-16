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

int	init_env(char **envp, t_env **head)
{
	int	i;
	
	i = 0;
	while (envp[i] != NULL)
	{
		if(add_env_var(envp[i], head) != 0)
		{
			free_env(head);
			return (ERR_MALLOC);
		}
		i++;
	}
	return (0);
}

int	add_env_var(char *str, t_env **head)
{
	char	*eq_pos;
	int	cut;
	char	*value;
	char	*key;
	t_env *new;

	key = NULL;
	value = NULL;
	eq_pos = ft_strchr(str, '=');
	if (!eq_pos)
	{
		key = ft_strdup(str);
		if (!key)
			return (ERR_MALLOC);
	}
	else
	{
		cut = eq_pos - str;
		key = ft_substr(str, 0, cut);
		value = ft_substr(str, cut + 1, ft_strlen(str) - cut - 1);
		if (!key || !value)
		{
			free(key);
			free(value);
			return (ERR_MALLOC);
		}
	}
	new = env_new(key, value, 1);
	if (!new)
	{
		free(key);
		free(value);
		return (ERR_MALLOC);
	}
	env_add_back(head, new);
	return (0);
}

