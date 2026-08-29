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
	int	ret;

	while (tokens != NULL)
	{
		tmp = command_new();
		if (!tmp)
		{
			free_commands(head);
			return (ERR_MALLOC);
		}
		command_add_back(head, tmp);
		ret = fill_command(&tokens, tmp);
		if (ret != 0)
		{
			free_commands(head);
			return (ret);
		}
		if (tokens != NULL)
			tokens = tokens->next;
	}
	return (0);
}

int	add_redirect2(t_token *token, t_piece **fields)
{
	if (split_word(token->next, fields) == ERR_MALLOC)
		return (ERR_MALLOC);
	if (count_fields(*fields) != 1)
		return (cmd_error(get_redir_symbol(token->redir_type), 
	"ambiguous redirect", ERR_REDIR));
	return (0);
}

int	add_redirect(t_token *token, t_command *command)
{
	t_redirect	*new;
	t_piece	*fields;
	int	ret;

	fields = NULL;
	new = redirect_new(token->redir_type);
	if (!new)
		return (ERR_MALLOC);
	redirect_add_back(&command->redirs, new);
	if (token->redir_type == R_HEREDOC)
	{
		if (handle_heredoc(new, token) == ERR_MALLOC)
			return (ERR_MALLOC);
		return (0);
	}
	ret = add_redirect2(token, &fields);
	if (ret != 0)
	{
		free_pieces(fields);
		return (ret);
	}
	new->target = fields->content;
	fields->content = NULL;
	free_pieces(fields);
	return (0);
}

int	fill_command2(t_token **tokens, t_command *cmd, t_piece **fields)
{
	t_token	*curr_token;
	int	ret;

	curr_token = *tokens;
	while (curr_token != NULL && curr_token->type != PIPE)
	{
		if (curr_token->type == REDIRECT)
		{
			ret = add_redirect(curr_token, cmd);
			if (ret != 0)
				return (ret);
			curr_token = curr_token->next;
		}
		else
		{
			ret = split_word(curr_token, fields);
			if (ret != 0)
				return (ret);
		}
		curr_token = curr_token->next;
	}
	*tokens = curr_token;
	return (0);
}

int	fill_command(t_token **tokens, t_command *cmd)
{
	t_piece	*fields;
	int	ret;	

	fields = NULL;
	ret = fill_command2(tokens, cmd, &fields);
	if (ret != 0)
	{
		free_pieces(fields);
		return (ret);
	}
	ret = fields_to_argv(cmd, fields);
	if (ret != 0)
	{
		free_pieces(fields);
		return (ret);
	}
	free_pieces(fields);
	return (0);
}


