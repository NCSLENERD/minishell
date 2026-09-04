/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_expansion2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 12:00:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/09/04 12:00:00 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

char	*expand_code(t_shell *shell, int *i, char *acc)
{
	char	*stock;

	stock = ft_itoa(shell->exit_code);
	acc = expand_append(acc, stock);
	free(stock);
	if (!acc)
		return (NULL);
	*i = *i + 2;
	return (acc);
}

char	*expand_var(char *content, t_shell *shell, int *i, char *acc)
{
	char	*stock;
	int		start;

	*i = *i + 1;
	start = *i;
	while (ft_isalnum(content[*i]) || content[*i] == '_')
		*i = *i + 1;
	stock = ft_substr(content, start, (*i - start));
	if (!stock)
	{
		free(acc);
		return (NULL);
	}
	acc = expand_append(acc, get_env_value(shell->env, stock));
	free(stock);
	if (!acc)
		return (NULL);
	return (acc);
}

char	*expand_litteral(char *content, int *i, char *acc)
{
	int		start;
	char	*stock;

	start = *i;
	while (content[*i] && !is_expandable(content, *i))
		*i = *i + 1;
	stock = ft_substr(content, start, (*i - start));
	if (!stock)
	{
		free(acc);
		return (NULL);
	}
	acc = expand_append(acc, stock);
	free(stock);
	if (!acc)
		return (NULL);
	return (acc);
}

char	*expand_str(char *content, t_shell *shell)
{
	int		i;
	char	*acc;

	acc = ft_strdup("");
	if (!acc)
		return (NULL);
	i = 0;
	while (content[i])
	{
		if (is_expandable(content, i))
		{
			if (content[i + 1] == '?')
				acc = expand_code(shell, &i, acc);
			else if (ft_isalpha(content[i + 1]) || content[i + 1] == '_')
				acc = expand_var(content, shell, &i, acc);
		}
		else
			acc = expand_litteral(content, &i, acc);
		if (!acc)
			return (NULL);
	}
	return (acc);
}
