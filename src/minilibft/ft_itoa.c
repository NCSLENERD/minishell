#include "../../minishell.h"

static int	len_int(long n)
{
	int	len;

	if (n == 0)
		return (1);
	len = 0;
	if (n < 0)
	{
		n = n * -1;
		len++;
	}
	while (n > 0)
	{
		n = n / 10;
		len++;
	}
	return (len);
}

static char	*is_zero(void)
{
	char	*res;

	res = malloc(sizeof(char) * 2);
	if (!res)
		return (NULL);
	res[0] = '0';
	res[1] = '\0';
	return (res);
}

static char	*fill_res(long stock_nb, int len)
{
	char	*res;

	res = malloc(sizeof(char) * (len + 1));
	if (!res)
		return (NULL);
	if (stock_nb < 0)
	{
		stock_nb = stock_nb * -1;
		res[0] = '-';
	}
	res[len] = '\0';
	while (stock_nb > 0 && len >= 0)
	{
		res[--len] = (stock_nb % 10) + '0';
		stock_nb = stock_nb / 10;
	}
	return (res);
}

char	*ft_itoa(int n)
{
	char	*res;
	int		len;
	long	stock_nb;

	if (n == 0)
	{
		res = is_zero();
		return (res);
	}
	stock_nb = n;
	len = len_int(stock_nb);
	res = fill_res(stock_nb, len);
	return (res);
}