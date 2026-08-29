/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc2.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/30 10:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/30 10:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

char	*heredoc_path(void)
{
	char	*num;
	char	*path;

	num = ft_itoa(getpid());
	if (num == NULL)
		return (NULL);
	path = ft_strjoin("/tmp/.minishell_heredoc_", num);
	free(num);
	return (path);
}

int	fill_heredoc(int fd, t_redirect *redir, t_shell *shell)
{
	char	*line;

	line = readline("> ");
	while (line != NULL && is_delimiter(line, redir->target) == 0)
	{
		if (write_heredoc_line(fd, line, redir, shell) != 0)
		{
			free(line);
			return (ERR_MALLOC);
		}
		free(line);
		line = readline("> ");
	}
	if (line == NULL)
		heredoc_warning(redir->target);
	free(line);
	return (0);
}
