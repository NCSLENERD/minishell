/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_tab.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 21:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/22 21:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	count_env_var(t_env *env)
{
	t_env	*curr;
	int		count;

	curr = env;
	count = 0;
	while (curr != NULL)
	{
		if (curr->exported == 1 && curr->value != NULL)
			count++;
		curr = curr->next;
	}
	return (count);
}

char	*make_env_entry(t_env *var)
{
	char	*tmp;
	char	*entry;

	tmp = ft_strjoin(var->key, "=");
	if (!tmp)
		return (NULL);
	entry = ft_strjoin(tmp, var->value);
	free(tmp);
	return (entry);
}

int	fill_env_tab(char **tab, t_env *env)
{
	t_env	*curr;
	int		i;

	curr = env;
	i = 0;
	while (curr != NULL)
	{
		if (curr->exported == 1 && curr->value != NULL)
		{
			tab[i] = make_env_entry(curr);
			if (!tab[i])
			{
				tab[i] = NULL;
				return (ERR_MALLOC);
			}
			i++;
		}
		curr = curr->next;
	}
	tab[i] = NULL;
	return (0);
}

char	**env_to_tab(t_env *env)
{
	char	**tab;

	tab = malloc(sizeof(char *) * (count_env_var(env) + 1));
	if (!tab)
		return (NULL);
	if (fill_env_tab(tab, env) != 0)
	{
		free_argv(tab);
		return (NULL);
	}
	return (tab);
}
