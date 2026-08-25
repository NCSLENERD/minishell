/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_exit.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/25 14:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/25 14:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

void	clean_exit(t_shell *shell, unsigned char code)
{
	free_env(&shell->env);
	exit(code);
}

int	exit_error(char *arg)
{
	ft_putstr_fd("minishell: exit: ", STDERR_FILENO);
	ft_putstr_fd(arg, STDERR_FILENO);
	ft_putstr_fd(": numeric argument required\n", STDERR_FILENO);
	return (2);
}

int	parse_sign(char *s, int *i)
{
	*i = 0;
	if (s[0] == '+' || s[0] == '-')
	{
		*i = 1;
		if (s[0] == '-')
			return (-1);
	}
	return (1);
}

int	parse_exit_code(char *s, unsigned char *code)
{
	int					i;
	int					sign;
	unsigned long long	n;

	n = 0;
	sign = parse_sign(s, &i);
	if (s[i] == '\0')
		return (0);
	while (s[i] != '\0')
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		if (n > (9223372036854775807ULL - (s[i] - '0')) / 10)
			return (0);
		n = n * 10 + (s[i] - '0');
		i++;
	}
	*code = (unsigned char)(sign * (long long)(n % 256));
	return (1);
}

int	builtin_exit(t_command *cmd, t_shell *shell)
{
	unsigned char	code;

	ft_putstr_fd("exit\n", STDERR_FILENO);
	if (cmd->argv[1] == NULL)
		clean_exit(shell, (unsigned char)shell->exit_code);
	if (parse_exit_code(cmd->argv[1], &code) == 0)
	{
		exit_error(cmd->argv[1]);
		clean_exit(shell, 2);
	}
	if (cmd->argv[2] != NULL)
		return (cmd_error("exit", "too many arguments", 1));
	clean_exit(shell, code);
	return (0);
}
