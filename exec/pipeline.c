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

// Le pere n'execute rien : il ferme tout ce dont il n'a plus l'usage.
// Garder fds[1] ouvert empecherait le fils suivant de voir l'EOF.
// Renvoie le prev_fd du tour suivant, ou -1 pour la derniere commande.
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

// Fork, puis cote fils : branchement des pipes avant exec_child, qui
// ne revient jamais. Cote pere : renvoie le pid du fils lance.
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

// Attend tous les fils pour ne pas laisser de zombies, mais ne retient
// que le code de la derniere commande du pipeline. L'ordre de mort n'est
// pas garanti, d'ou la comparaison du pid retourne avec celui attendu.
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
