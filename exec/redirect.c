/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirect.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/23 11:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/23 11:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	open_redir(t_redirect *redir)
{
	int	fd;

	if (redir->type == R_IN)
	{
		fd = open(redir->target, O_RDONLY);
		return (fd);
	}
	if (redir->type == R_OUT)
	{
		fd = open(redir->target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		return (fd);
	}
	if (redir->type == R_APPEND)
	{
		fd = open(redir->target, O_WRONLY | O_CREAT | O_APPEND, 0644);
		return (fd);
	}
	if (redir->type == R_HEREDOC)
		return (redir->fd);
	return (-1);
}

int	apply_redirs(t_redirect *redirs)
{
	int			fd;
	t_redirect	*curr;

	curr = redirs;
	while (curr)
	{
		fd = open_redir(curr);
		if (fd < 0)
			return (cmd_error(curr->target, strerror(errno), 1));
		if (curr->type == R_IN || curr->type == R_HEREDOC)
			dup2(fd, STDIN_FILENO);
		else
			dup2(fd, STDOUT_FILENO);
		close(fd);
		if (curr->type == R_HEREDOC)
			curr->fd = -1;
		curr = curr->next;
	}
	return (0);
}
