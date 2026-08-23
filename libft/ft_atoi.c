/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 13:17:46 by rlaghsal          #+#    #+#             */
/*   Updated: 2025/11/13 19:46:21 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_is_space(char c)
{
	return (c == '\n' || c == '\t' || c == '\r'
		|| c == ' ' || c == '\v' || c == '\f');
}

int	ft_atoi(const char *nptr)
{
	int	sign;
	int	i;
	int	n;

	i = 0;
	n = 0;
	sign = 1;
	while (ft_is_space(nptr[i]))
		i++;
	if (nptr[i] == '-' || nptr[i] == '+')
	{
		if (nptr[i] == '-')
			sign *= -1;
		i++;
	}
	while (nptr[i] >= '0' && nptr[i] <= '9')
	{
		n *= 10;
		n += nptr[i] - '0';
		i++;
	}
	n *= sign;
	return (n);
}
