/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_lexer2.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 21:14:50 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/15 21:14:51 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

char	get_backslash_char(char c)
{
	const char	*esc;
	const char	*val;
	int			i;

	if (c == '0')
		return ('\0');
	esc = "ntrvfabe\\'\"";
	val = "\n\t\r\v\f\a\b\e\\'\"";
	i = 0;
	while (esc[i])
	{
		if (esc[i] == c)
			return (val[i]);
		i++;
	}
	return (-1);
}

char	*trad_backslash(char *content)
{
	char	*res;
	int		i;
	int		j;

	res = malloc(sizeof(char) * (ft_strlen(content) + 1));
	if (!res)
		return (NULL);
	i = 0;
	j = 0;
	while (content[i])
	{
		if (content[i] == '\\' && content[i + 1] != '\0')
			trad_backslash2(content, res, &i, &j);
		else
		{
			res[j] = content[i];
			j++;
			i++;
		}
	}
	res[j] = '\0';
	return (res);
}

void	trad_backslash2(char *content, char *res, int *i, int *j)
{
	char	c;

	c = get_backslash_char(content[*i + 1]);
	if (c != -1)
	{
		res[*j] = c;
		*j = *j + 1;
		*i = *i + 2;
	}
	else
	{
		res[*j] = content[*i];
		*j = *j + 1;
		*i = *i + 1;
	}
}
