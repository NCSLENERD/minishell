/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipeline.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/23 12:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/23 12:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

void	child_pipe_setup(int prev_fd, int *fds, t_command *cmd)
{
	if (prev_fd != -1)
	{
		dup2(prev_fd, STDIN_FILENO);
		close(prev_fd);
	}
	if (cmd->next != NULL)
	{
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]);
	}
}

int	parent_pipe_setup(int prev_fd, int *fds, t_command *cmd)
{
	if (prev_fd != -1)
		close(prev_fd);
	if (cmd->next != NULL)
	{
		close(fds[1]);
		return (fds[0]);
	}
	return (-1);
}

int	run_pipeline(t_command *cmds, t_shell *shell)
{
	t_command	*curr;
	int			fds[2];
	int			prev_fd;
	pid_t		pid;

	prev_fd = -1;
	curr = cmds;
	while (curr != NULL)
	{
		if (curr->next != NULL && pipe(fds) == -1)
			return (cmd_error("pipe", strerror(errno), 1));
		pid = launch_command(curr, shell, prev_fd, fds);
		if (pid == -1)
			return (1);
		prev_fd = parent_pipe_setup(prev_fd, fds, curr);
		curr = curr->next;
	}
	wait_all(pid, shell);
	return (0);
}

pid_t	launch_command(t_command *cmd, t_shell *shell, int prev_fd, int *fds)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		return (-1);
	}
	if (pid == 0)
	{
		child_pipe_setup(prev_fd, fds, cmd);
		exec_child(cmd, shell);
	}
	return (pid);
}

void	wait_all(pid_t last, t_shell *shell)
{
	pid_t	pid;
	int		status;

	pid = waitpid(-1, &status, 0);
	while (pid > 0)
	{
		if (pid == last)
			set_exit_status(shell, status);
		pid = waitpid(-1, &status, 0);
	}
}
