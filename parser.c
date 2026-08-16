/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 16:00:43 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/16 16:00:46 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "minishell.h"

int	parser(t_token *tokens, t_command **head)
{
	t_command	*tmp;

	while (tokens != NULL)
	{
		tmp = command_new();
		if (!tmp)
		{
			free_commands(head);
			return (ERR_MALLOC);
		}
		command_add_back(head, tmp);
		if (fill_command(&tokens, tmp) == ERR_MALLOC)
		{
			free_commands(head);
			return (ERR_MALLOC);
		}
		if (tokens != NULL)
			tokens = tokens->next;
	}
	return (0);
}

int	count_argv(t_token *tokens)
{
	int	count;

	count = 0;
	while (tokens != NULL && tokens->type != PIPE)
	{
		if (tokens->type == REDIRECT)
			tokens = tokens->next;
		else
			count++;
		tokens = tokens->next;
	}
	return (count);
}

int	add_redirect(t_token *token, t_command *command)
{
	t_redirect *new;
	t_piece	*tmp;

	new = redirect_new(token->redir_type);
	if (!new)
		return (ERR_MALLOC);
	redirect_add_back(&command->redirs, new);
	new->target = piece_to_str(token->next->piece);
	if (!new->target)
		return (ERR_MALLOC);
	tmp = token->next->piece;
	while (tmp != NULL && new->flag_quote == 0)
	{
		if (tmp->quote != Q_NONE)
			new->flag_quote = 1;
		tmp = tmp->next;
	}
	return (0);
}

int	fill_command(t_token **tokens, t_command *cmd)
{
	int	count;
	t_token	*curr_token;
	int	i;

	i = 0;
	curr_token = *tokens;
	count = count_argv(*tokens);
	cmd->argv = malloc(sizeof(char *) * (count + 1));
	if (!cmd->argv)
		return (ERR_MALLOC);
	while (curr_token != NULL && curr_token->type != PIPE)
	{
		if (curr_token->type == REDIRECT)
		{
			if(add_redirect(curr_token, cmd) != 0)
			{
				cmd->argv[i] = NULL;
				return (ERR_MALLOC);
			}
			curr_token = curr_token->next;
		}
		else
		{
			cmd->argv[i] = piece_to_str(curr_token->piece);
			if (!cmd->argv[i])
			{
				cmd->argv[i] = NULL;
				return (ERR_MALLOC);
			}
			i++;
		}
		curr_token = curr_token->next;
	}
	cmd->argv[i] = NULL;
	*tokens = curr_token;
	return (0);
}

// > out echo hi