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

// Ligne vide, ou commande unique sans nom et sans redirection : rien
// a lancer. Une redirection seule (> fichier) doit en revanche etre
// executee, elle cree le fichier.
int	nothing_to_do(t_command *cmds)
{
	if (cmds == NULL || cmds->argv == NULL)
		return (1);
	if (cmds->next == NULL && cmds->argv[0] == NULL && cmds->redirs == NULL)
		return (1);
	return (0);
}

// Lance les commandes du pipeline en parallele, puis les attend toutes.
// Le pipe est cree avant le fork pour que les deux processus en heritent,
// et seulement s'il reste une commande apres celle-ci.
int	execute(t_command *cmds, t_shell *shell)
{
	t_command	*curr;
	int			fds[2];
	int			prev_fd;
	pid_t		pid;

	if (nothing_to_do(cmds))
		return (0);
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

void	set_exit_status(t_shell *shell, int status)
{
	if (WIFEXITED(status))
		shell->exit_code = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		shell->exit_code = 128 + WTERMSIG(status);
}
