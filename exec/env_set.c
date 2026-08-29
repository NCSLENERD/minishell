/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_set.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 17:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/29 17:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	add_new_env(t_env **env, char *key, char *value)
{
	t_env	*new;
	char	*dup_key;

	dup_key = ft_strdup(key);
	if (dup_key == NULL)
	{
		free(value);
		return (ERR_MALLOC);
	}
	new = env_new(dup_key, value, 1);
	if (new == NULL)
	{
		free(dup_key);
		free(value);
		return (ERR_MALLOC);
	}
	env_add_back(env, new);
	return (0);
}

int	set_env_value(t_env **env, char *key, char *value)
{
	t_env	*curr;
	char	*dup_value;

	dup_value = ft_strdup(value);
	if (dup_value == NULL)
		return (ERR_MALLOC);
	curr = *env;
	while (curr != NULL)
	{
		if (ft_strncmp(curr->key, key, ft_strlen(key) + 1) == 0)
		{
			free(curr->value);
			curr->value = dup_value;
			curr->exported = 1;
			return (0);
		}
		curr = curr->next;
	}
	return (add_new_env(env, key, dup_value));
}
