/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/11 15:15:07 by rlaghsal          #+#    #+#             */
/*   Updated: 2025/11/15 02:22:01 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t siz)
{
	size_t	i;
	size_t	to_start;
	size_t	src_len;
	size_t	dst_len;

	i = 0;
	to_start = 0;
	src_len = ft_strlen(src);
	dst_len = ft_strlen(dst);
	while (dst[to_start])
		to_start++;
	while (src[i] && to_start + i + 1 < siz)
	{
		dst[to_start + i] = src[i];
		i++;
	}
	if (siz > i)
		dst[to_start + i] = '\0';
	if (siz > dst_len)
		return (src_len + dst_len);
	return (src_len + siz);
}
