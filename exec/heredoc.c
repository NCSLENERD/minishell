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
	int		p[2];
	char	*line;

	if (pipe(p) == -1)
		return (cmd_error("heredoc", strerror(errno), ERR_MALLOC));
	line = readline("> ");
	while (line != NULL && is_delimiter(line, redir->target) == 0)
	{
		if (write_heredoc_line(p[1], line, redir, shell) != 0)
		{
			free(line);
			close(p[0]);
			close(p[1]);
			return (ERR_MALLOC);
		}
		free(line);
		line = readline("> ");
	}
	if (line == NULL)
		heredoc_warning(redir->target);
	free(line);
	close(p[1]);
	redir->fd = p[0];
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
