/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_cd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 17:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/29 17:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	cd_error(char *arg, char *msg)
{
	ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
	ft_putstr_fd(arg, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putstr_fd(msg, STDERR_FILENO);
	ft_putstr_fd("\n", STDERR_FILENO);
	return (1);
}

char	*get_cd_target(t_command *cmd, t_shell *shell)
{
	char	*target;

	if (cmd->argv[1] == NULL)
	{
		target = get_env_value(shell->env, "HOME");
		if (target == NULL || target[0] == '\0')
		{
			cmd_error("cd", "HOME not set", 1);
			return (NULL);
		}
		return (target);
	}
	if (ft_strncmp(cmd->argv[1], "-", 2) == 0)
	{
		target = get_env_value(shell->env, "OLDPWD");
		if (target == NULL)
		{
			cmd_error("cd", "OLDPWD not set", 1);
			return (NULL);
		}
		return (target);
	}
	return (cmd->argv[1]);
}

int	update_pwd(t_shell *shell, char *oldpwd)
{
	char	*cwd;

	if (oldpwd != NULL && set_env_value(&shell->env, "OLDPWD", oldpwd) != 0)
		return (ERR_MALLOC);
	cwd = getcwd(NULL, 0);
	if (cwd == NULL)
		return (0);
	if (set_env_value(&shell->env, "PWD", cwd) != 0)
	{
		free(cwd);
		return (ERR_MALLOC);
	}
	free(cwd);
	return (0);
}

int	builtin_cd(t_command *cmd, t_shell *shell)
{
	char	*target;
	char	*oldpwd;

	if (cmd->argv[1] != NULL && cmd->argv[2] != NULL)
		return (cmd_error("cd", "too many arguments", 1));
	target = get_cd_target(cmd, shell);
	if (target == NULL)
		return (1);
	oldpwd = getcwd(NULL, 0);
	if (chdir(target) != 0)
	{
		free(oldpwd);
		return (cd_error(target, strerror(errno)));
	}
	if (cmd->argv[1] != NULL && ft_strncmp(cmd->argv[1], "-", 2) == 0)
	{
		ft_putstr_fd(target, STDOUT_FILENO);
		ft_putstr_fd("\n", STDOUT_FILENO);
	}
	update_pwd(shell, oldpwd);
	free(oldpwd);
	return (0);
}
