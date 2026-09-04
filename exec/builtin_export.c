/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_export.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 12:00:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/09/04 12:00:00 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../minishell.h"

int	builtin_export(t_command *cmd, t_shell *shell)
{
	int	i;
	int	ret;

	if (cmd->argv[1] == NULL)
		return (print_export(shell->env));
	i = 1;
	ret = 0;
	while (cmd->argv[i] != NULL)
	{
		if (is_valid_identifier(cmd->argv[i]) == 0)
			ret = export_error(cmd->argv[i]);
		else if (export_one(cmd->argv[i], shell) == ERR_MALLOC)
			return (ERR_MALLOC);
		i++;
	}
	return (ret);
}

int	is_valid_identifier(char *arg)
{
	int	i;

	if (!ft_isalpha(arg[0]) && arg[0] != '_')
		return (0);
	i = 1;
	while (arg[i] && arg[i] != '=')
	{
		if (ft_isalnum(arg[i]) || arg[i] == '_')
			i++;
		else
			return (0);
	}
	return (1);
}

int	export_one2(t_shell *shell, char *key)
{
	t_env	*ret_key;
	int		ret;

	ret = 0;
	if (key == NULL)
		return (ERR_MALLOC);
	ret_key = find_env_key(&shell->env, key);
	if (ret_key)
		ret_key->exported = 1;
	else
	{
		ret = add_new_env(&shell->env, key, NULL);
		if (ret == ERR_MALLOC)
			return (ERR_MALLOC);
	}
	return (0);
}

int	export_one(char *arg, t_shell *shell)
{
	char	*key;
	char	*value;
	int		ret;

	ret = 0;
	value = ft_strchr(arg, '=');
	if (value)
		key = ft_substr(arg, 0, value - arg);
	else
		key = ft_substr(arg, 0, ft_strlen(arg));
	if (key == NULL)
		return (ERR_MALLOC);
	if (!value)
	{
		ret = export_one2(shell, key);
		free(key);
		return (ret);
	}
	if (set_env_value(&shell->env, key, value + 1) == ERR_MALLOC)
	{
		free(key);
		return (ERR_MALLOC);
	}
	free(key);
	return (0);
}

int	export_error(char *arg)
{
	ft_putstr_fd("minishell: export: `", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd("': not a valid identifier\n", 2);
	return (1);
}
