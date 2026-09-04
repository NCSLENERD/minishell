/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_envp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 12:00:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/09/04 12:00:00 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

int	init_env(char **envp, t_env **head)
{
	int	i;

	i = 0;
	while (envp[i] != NULL)
	{
		if (add_env_var(envp[i], head) != 0)
		{
			free_env(head);
			return (ERR_MALLOC);
		}
		i++;
	}
	return (0);
}

int	parse_var_env(char *str, char **key, char **value)
{
	char	*eq_pos;
	int		cut;

	*key = NULL;
	*value = NULL;
	eq_pos = ft_strchr(str, '=');
	if (!eq_pos)
	{
		*key = ft_strdup(str);
		if (!(*key))
			return (ERR_MALLOC);
	}
	else
	{
		cut = eq_pos - str;
		*key = ft_substr(str, 0, cut);
		*value = ft_substr(str, cut + 1, ft_strlen(str) - cut - 1);
		if (!(*key) || !(*value))
		{
			free(*key);
			free(*value);
			return (ERR_MALLOC);
		}
	}
	return (0);
}

int	add_env_var(char *str, t_env **head)
{
	char	*value;
	char	*key;
	int		ret;
	t_env	*new;

	ret = parse_var_env(str, &key, &value);
	if (ret != 0)
		return (ret);
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
