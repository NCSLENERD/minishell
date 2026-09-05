/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_cd2.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rlaghsal <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/05 12:00:00 by rlaghsal          #+#    #+#             */
/*   Updated: 2026/09/05 12:00:00 by rlaghsal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	cd_is_tilde(char *arg)
{
	if (arg == NULL || arg[0] != '~')
		return (0);
	return (arg[1] == '\0' || arg[1] == '/');
}

char	*cd_tilde_path(char *arg, t_shell *shell)
{
	char	*home;

	home = get_env_value(shell->env, "HOME");
	if (home == NULL || home[0] == '\0')
	{
		cmd_error("cd", "HOME not set", 1);
		return (NULL);
	}
	if (arg == NULL || arg[1] == '\0')
		return (ft_strdup(home));
	return (ft_strjoin(home, arg + 1));
}
