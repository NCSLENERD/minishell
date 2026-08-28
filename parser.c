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

int	fill_command2(t_token **tokens, t_command *cmd, t_piece **fields)
{
	t_token	*curr_token;

	curr_token = *tokens;
	while (curr_token != NULL && curr_token->type != PIPE)
	{
		if (curr_token->type == REDIRECT)
		{
			if(add_redirect(curr_token, cmd) != 0)
				return (ERR_MALLOC);
			curr_token = curr_token->next;
		}
		else
		{
			if (split_word(curr_token, fields) == ERR_MALLOC)
				return (ERR_MALLOC);
		}
		curr_token = curr_token->next;
	}
	*tokens = curr_token;
	return (0);
}

int	fill_command(t_token **tokens, t_command *cmd)
{
	t_piece	*fields;

	fields = NULL;
	if (fill_command2(tokens, cmd, &fields) == ERR_MALLOC)
	{
		free_pieces(fields);
		return (ERR_MALLOC);
	}
	if (fields_to_argv(cmd, fields) == ERR_MALLOC)
	{
		free_pieces(fields);
		return (ERR_MALLOC);
	}
	free_pieces(fields);
	return (0);
}


