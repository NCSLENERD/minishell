/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 15:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/29 15:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	is_delimiter(char *line, char *delim)
{
	return (ft_strncmp(line, delim, ft_strlen(delim) + 1) == 0);
}

int	write_heredoc_line(int fd, char *line, t_redirect *redir, t_shell *shell)
{
	char	*expanded;

	if (redir->flag_quote == 1)
	{
		ft_putstr_fd(line, fd);
		ft_putstr_fd("\n", fd);
		return (0);
	}
	expanded = expand_str(line, shell);
	if (expanded == NULL)
		return (ERR_MALLOC);
	ft_putstr_fd(expanded, fd);
	ft_putstr_fd("\n", fd);
	free(expanded);
	return (0);
}

int	read_heredoc(t_redirect *redir, t_shell *shell)
{
	char	*path;
	int		fd;
	int		ret;

	path = heredoc_path();
	if (path == NULL)
		return (ERR_MALLOC);
	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1)
	{
		free(path);
		return (cmd_error("heredoc", strerror(errno), ERR_MALLOC));
	}
	ret = fill_heredoc(fd, redir, shell);
	close(fd);
	if (ret == 0)
		redir->fd = open(path, O_RDONLY);
	unlink(path);
	free(path);
	if (ret != 0)
		return (ret);
	if (redir->fd == -1)
		return (cmd_error("heredoc", strerror(errno), ERR_MALLOC));
	return (0);
}

int	collect_heredocs(t_command *cmds, t_shell *shell)
{
	t_command	*cmd;
	t_redirect	*redir;

	cmd = cmds;
	while (cmd != NULL)
	{
		redir = cmd->redirs;
		while (redir != NULL)
		{
			if (redir->type == R_HEREDOC && read_heredoc(redir, shell) != 0)
				return (ERR_MALLOC);
			redir = redir->next;
		}
		cmd = cmd->next;
	}
	return (0);
}

void	close_heredocs(t_command *cmds)
{
	t_command	*cmd;
	t_redirect	*redir;

	cmd = cmds;
	while (cmd != NULL)
	{
		redir = cmd->redirs;
		while (redir != NULL)
		{
			if (redir->type == R_HEREDOC && redir->fd >= 0)
			{
				close(redir->fd);
				redir->fd = -1;
			}
			redir = redir->next;
		}
		cmd = cmd->next;
	}
}
