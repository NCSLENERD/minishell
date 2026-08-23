/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 00:17:06 by rlaghsal          #+#    #+#             */
/*   Updated: 2025/11/15 03:10:28 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	to_split(const char *s, int index, char c)
{
	int	i;

	i = 0;
	while (s[i + index] && s[i + index] != c)
		i++;
	return (i);
}

static void	to_fill(char *out, const char *s, int *i, size_t len)
{
	size_t	j;

	j = 0;
	while (j < len)
		out[j++] = s[(*i)++];
	out[j] = '\0';
}

char	**ft_split(char const *s, char c)
{
	char	**out;
	int		i;
	int		k;

	out = malloc(sizeof(char *) * (ft_strlen(s) + 1));
	if (!out)
		return (NULL);
	i = 0;
	k = 0;
	while (s[i])
	{
		while (s[i] && s[i] == c)
			i++;
		if (s[i] && s[i] != c)
		{
			out[k] = malloc(sizeof(char) * to_split(s, i, c) + 1);
			if (!out[k])
				return (NULL);
			to_fill(out[k++], s, &i, to_split(s, i, c));
		}
	}
	out[k] = NULL;
	return (out);
}
/*
#include <stdio.h>
int	main()
{
	char **result;
    char const *str = "";
    char sep = ' ';
    int i = 0;

    result = ft_split(str, sep);
}*/
