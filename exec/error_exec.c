/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_exec.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/23 10:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/23 10:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	cmd_error(char *cmd, char *msg, int code)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd(cmd, 2);
	ft_putstr_fd(": ", 2);
	ft_putstr_fd(msg, 2);
	ft_putstr_fd("\n", 2);
	return (code);
}

int	execve_error(char *cmd)
{
	if (errno == EACCES)
		return (cmd_error(cmd, "Permission denied", 126));
	if (errno == ENOEXEC)
		return (cmd_error(cmd, "Exec format error", 126));
	if (errno == ENOTDIR)
		return (cmd_error(cmd, "Not a directory", 126));
	if (errno == ENOENT)
		return (cmd_error(cmd, "No such file or directory", 127));
	return (cmd_error(cmd, strerror(errno), 1));
}
