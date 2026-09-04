/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_export2.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 12:00:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/09/04 12:00:00 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../minishell.h"

void	display_export(t_env **tab)
{
	int	i;

	i = 0;
	while (tab[i])
	{
		if (tab[i]->value == NULL)
		{
			ft_putstr_fd("declare -x ", STDOUT_FILENO);
			ft_putstr_fd(tab[i]->key, STDOUT_FILENO);
			ft_putstr_fd("\n", STDOUT_FILENO);
		}
		else
		{
			ft_putstr_fd("declare -x ", STDOUT_FILENO);
			ft_putstr_fd(tab[i]->key, STDOUT_FILENO);
			ft_putstr_fd("=", STDOUT_FILENO);
			ft_putstr_fd("\"", STDOUT_FILENO);
			ft_putstr_fd(tab[i]->value, STDOUT_FILENO);
			ft_putstr_fd("\"", STDOUT_FILENO);
			ft_putstr_fd("\n", STDOUT_FILENO);
		}
		i++;
	}
	return ;
}

void	sort_export(t_env **tab)
{
	int		i;
	int		j;
	t_env	*tmp;

	i = 0;
	while (tab[i])
	{
		j = i;
		while (tab[j])
		{
			if (ft_strncmp(tab[i]->key, tab[j]->key,
					ft_strlen(tab[j]->key) + 1) > 0)
			{
				tmp = tab[j];
				tab[j] = tab[i];
				tab[i] = tmp;
			}
			j++;
		}
		i++;
	}
}

int	print_export(t_env *env)
{
	t_env	**tab;

	tab = fill_env_tab_var_exported(env);
	if (!tab)
		return (ERR_MALLOC);
	sort_export(tab);
	display_export(tab);
	free(tab);
	return (0);
}

t_env	**fill_env_tab_var_exported(t_env *env)
{
	t_env	**tab;
	t_env	*curr;
	int		i;

	curr = env;
	i = 0;
	tab = malloc(sizeof(t_env *) * (count_env_var_exported(env) + 1));
	if (!tab)
		return (NULL);
	while (curr)
	{
		if (curr->exported == 1)
		{
			tab[i] = curr;
			i++;
		}
		curr = curr->next;
	}
	tab[i] = NULL;
	return (tab);
}

int	count_env_var_exported(t_env *env)
{
	t_env	*curr;
	int		count;

	curr = env;
	count = 0;
	while (curr != NULL)
	{
		if (curr->exported == 1)
			count++;
		curr = curr->next;
	}
	return (count);
}
