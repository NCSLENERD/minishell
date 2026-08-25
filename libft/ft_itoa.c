/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/13 13:01:39 by rlaghsal          #+#    #+#             */
/*   Updated: 2025/11/15 13:01:41 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	count_size(long nb)
{
	size_t	size;

	size = 0;
	if (nb < 0)
	{
		nb *= -1;
		size = 1;
	}
	if (nb == 0)
		size = 1;
	else
	{
		while (nb)
		{
			nb /= 10;
			size++;
		}
	}
	return (size);
}

static void	recursive_itoa(long n, int *i, char *out)
{
	if (n >= 10)
		recursive_itoa(n / 10, i, out);
	out[(*i)++] = n % 10 + '0';
}

char	*ft_itoa(int n)
{
	long	nb;
	char	*out;
	int		i;

	nb = n;
	out = malloc(sizeof(char) * (count_size(nb) + 1));
	if (out == NULL)
		return (NULL);
	i = 0;
	if (nb < 0)
	{
		out[i++] = '-';
		nb *= -1;
	}
	recursive_itoa(nb, &i, out);
	out[i] = '\0';
	return (out);
}
/*int	main(void)
{
	char	*out;
	int	n = -545;
	out = ft_itoa(n);
	printf("%s", out);
}*/
