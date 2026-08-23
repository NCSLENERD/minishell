/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 00:20:04 by rlaghsal          #+#    #+#             */
/*   Updated: 2025/11/15 03:30:03 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	is_in_set(char c, const char *set)
{
	int	i;

	i = 0;
	while (set[i])
	{
		if (c == set[i])
			return (1);
		i++;
	}
	return (0);
}

char	*ft_strtrim(char const *s1, char const *set)
{
	char	*new;
	int		i;
	int		j;
	int		k;

	if (!s1 || !set)
		return (NULL);
	i = 0;
	k = 0;
	j = ft_strlen(s1) - 1;
	while (s1[i] && is_in_set(s1[i], set))
		i++;
	while (s1[j] && is_in_set(s1[j], set))
		j--;
	if (j < 0)
		j = i;
	new = malloc(sizeof(char) * (j - i + 2));
	if (!new)
		return (NULL);
	while (s1[i] && i <= j)
		new[k++] = s1[i++];
	new[k] = '\0';
	return (new);
}
