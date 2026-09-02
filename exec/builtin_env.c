/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_env.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/25 13:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/08/25 13:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	builtin_pwd(void)
{
	char	*cwd;

	cwd = getcwd(NULL, 0);
	if (cwd == NULL)
		return (cmd_error("pwd", strerror(errno), 1));
	ft_putstr_fd(cwd, STDOUT_FILENO);
	ft_putstr_fd("\n", STDOUT_FILENO);
	free(cwd);
	return (0);
}

int	builtin_env(t_env *env)
{
	t_env	*curr;

	curr = env;
	while (curr != NULL)
	{
		if (curr->exported == 1 && curr->value != NULL)
		{
			ft_putstr_fd(curr->key, STDOUT_FILENO);
			ft_putstr_fd("=", STDOUT_FILENO);
			ft_putstr_fd(curr->value, STDOUT_FILENO);
			ft_putstr_fd("\n", STDOUT_FILENO);
		}
		curr = curr->next;
	}
	return (0);
}

t_env   *find_env_key(t_env **env, char *key)
{
    t_env   *curr;

    curr = *env;
    while (curr)
    {
        if (ft_strncmp(key, curr->key, ft_strlen(key) + 1) == 0)
            return (curr);
        curr = curr->next;
    }
    return (NULL);
}
