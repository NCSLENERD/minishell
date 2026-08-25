/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_echo.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/25 13:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/25 13:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	is_n_flag(char *s)
{
	int	i;

	if (s[0] != '-' || s[1] != 'n')
		return (0);
	i = 1;
	while (s[i] == 'n')
		i++;
	return (s[i] == '\0');
}

int	builtin_echo(char **argv)
{
	int	i;
	int	newline;

	i = 1;
	newline = 1;
	while (argv[i] != NULL && is_n_flag(argv[i]))
	{
		newline = 0;
		i++;
	}
	while (argv[i] != NULL)
	{
		ft_putstr_fd(argv[i], STDOUT_FILENO);
		if (argv[i + 1] != NULL)
			ft_putstr_fd(" ", STDOUT_FILENO);
		i++;
	}
	if (newline == 1)
		ft_putstr_fd("\n", STDOUT_FILENO);
	return (0);
}
