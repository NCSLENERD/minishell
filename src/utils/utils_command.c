/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_command.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 12:00:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/09/04 12:00:00 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

t_command	*command_new(void)
{
	t_command	*new;

	new = malloc(sizeof(t_command));
	if (!new)
		return (NULL);
	new->argv = NULL;
	new->redirs = NULL;
	new->next = NULL;
	return (new);
}

void	command_add_back(t_command **head, t_command *command)
{
	t_command	*curr;

	if (*head == NULL)
	{
		*head = command;
		return ;
	}
	curr = *head;
	while (curr != NULL)
	{
		if (curr->next == NULL)
		{
			curr->next = command;
			break ;
		}
		curr = curr->next;
	}
}

void	free_argv(char **argv)
{
	int	i;

	if (!argv)
		return ;
	i = 0;
	while (argv[i] != NULL)
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}

void	free_commands(t_command **head)
{
	t_command	*curr1;
	t_command	*curr2;

	curr1 = *head;
	curr2 = NULL;
	while (curr1 != NULL)
	{
		curr2 = curr1->next;
		free_redirects(curr1->redirs);
		free_argv(curr1->argv);
		free(curr1);
		curr1 = curr2;
	}
	*head = NULL;
}
