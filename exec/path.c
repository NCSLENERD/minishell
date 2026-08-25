/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   path.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 21:30:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/22 21:30:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

char	*get_env_value(t_env *env, char *key)
{
	t_env	*curr;

	curr = env;
	while (curr)
	{
		if (ft_strncmp(curr->key, key, ft_strlen(key) + 1) == 0)
			return (curr->value);
		curr = curr->next;
	}
	return (NULL);
}

char	*join_path(char *dir, char *cmd)
{
	char	*path;
	char	*tmp;

	tmp = ft_strjoin(dir, "/");
	if (!tmp)
		return (NULL);
	path = ft_strjoin(tmp, cmd);
	free(tmp);
	return (path);
}

char	*search_in_path(char **dir, char *cmd)
{
	char	*full;
	int		i;

	i = 0;
	while (dir[i])
	{
		full = join_path(dir[i], cmd);
		if (!full)
			return (NULL);
		if (!access(full, X_OK))
			return (full);
		free(full);
		i++;
	}
	return (NULL);
}

char	*find_path(char *cmd, t_env *env)
{
	char	**dirs;
	char	*paths;
	char	*full;

	if (cmd[0] == '\0')
		return (NULL);
	if (ft_strchr(cmd, '/') != NULL)
		return (ft_strdup(cmd));
	paths = get_env_value(env, "PATH");
	if (paths == NULL || paths[0] == '\0')
		return (NULL);
	dirs = ft_split(paths, ':');
	if (!dirs)
		return (NULL);
	full = search_in_path(dirs, cmd);
	free_argv(dirs);
	return (full);
}
