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
	if (is_builtin(cmd->argv[0]))
		exit(run_builtin(cmd, shell));
	envp = env_to_tab(shell->env);
	if (!envp)
		exit(cmd_error(cmd->argv[0], "allocation error", 1));
	path = find_path(cmd->argv[0], shell->env);
	if (!path)
		exit(cmd_error(cmd->argv[0], "command not found", 127));
	execve(path, cmd->argv, envp);
	exit(execve_error(cmd->argv[0]));
}

int	nothing_to_do(t_command *cmds)
{
	if (cmds == NULL || cmds->argv == NULL)
		return (1);
	if (cmds->next == NULL && cmds->argv[0] == NULL && cmds->redirs == NULL)
		return (1);
	return (0);
}

int	execute(t_command *cmds, t_shell *shell)
{
	int	ret;
	int	ret_collect;

	if (nothing_to_do(cmds))
		return (0);
	ret_collect = collect_heredocs(cmds, shell);
	if (ret_collect == CANCEL_HEREDOC)
	{
		shell->exit_code = 128 + SIGINT;
		close_heredocs(cmds);
		return (0);
	}
	if (ret_collect != 0)
		return (ret_collect);
	ret = 0;
	if (cmds->next == NULL && is_builtin(cmds->argv[0]))
		shell->exit_code = run_builtin_parent(cmds, shell);
	else
		ret = run_pipeline(cmds, shell);
	close_heredocs(cmds);
	return (ret);
}

void	set_exit_status(t_shell *shell, int status)
{
	if (WIFEXITED(status))
		shell->exit_code = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
	{
		shell->exit_code = 128 + WTERMSIG(status);
		if (WTERMSIG(status) == SIGINT)
			write(1, "\n", 1);
		else if (WTERMSIG(status) == SIGQUIT)
			write(1, "Quit: 3\n", 8);
	}
}
