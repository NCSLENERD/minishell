/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 20:08:28 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/22 20:15:32 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

void	exec_child(t_command *cmd, t_shell *shell)
{
	char	**envp;
	char	*path;

	if (apply_redirs(cmd->redirs) != 0)
		exit(1);
	if (cmd->argv[0] == NULL)
		exit(0);
	envp = env_to_tab(shell->env);
	if (!envp)
		exit(cmd_error(cmd->argv[0], "allocation error", 1));
	path = find_path(cmd->argv[0], shell->env);
	if (!path)
		exit(cmd_error(cmd->argv[0], "command not found", 127));
	execve(path, cmd->argv, envp);
	exit(execve_error(cmd->argv[0]));
}

int	execute(t_command *cmd, t_shell *shell)
{
	pid_t	pid;
	int		status;

	if (!cmd || !cmd->argv)
		return (0);
	if (cmd->argv[0] == NULL && cmd->redirs == NULL)
		return (0);
	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		return (1);
	}
	if (pid == 0)
		exec_child(cmd, shell);
	if (waitpid(pid, &status, 0) != -1)
		set_exit_status(shell, status);
	return (0);
}

void	set_exit_status(t_shell *shell, int status)
{
	if (WIFEXITED(status))
		shell->exit_code = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		shell->exit_code = 128 + WTERMSIG(status);
}
