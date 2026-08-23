/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 00:33:57 by rlaghsal          #+#    #+#             */
/*   Updated: 2025/11/15 03:39:35 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t count, size_t size)
{
	void	*new;
	size_t	i;

	new = (void *) malloc(count * size);
	if (!new)
		return (NULL);
	i = 0;
	while (i < count * size)
	{
		*(unsigned char *)(new + i) = 0;
		i++;
	}
	return (new);
}
