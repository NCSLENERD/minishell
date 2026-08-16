/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/15 17:58:57 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/15 17:58:58 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "minishell.h"

void	print_token(t_token *token)
{
	t_piece *curr_piece;
	t_token *curr_token;

	if (token == NULL)
		return;
	curr_token = token;
	while(curr_token != NULL)
	{
		curr_piece = curr_token->piece;
		if (curr_token->type == MOT)
			printf("TYPE: MOT ");
		else if (curr_token->type == PIPE)
			printf("TYPE: PIPE ");
		else if (curr_token->type == REDIRECT)
			printf("TYPE: REDIRECT ");
		else 
			printf("TYPE: ERROR(& && or ;) ");
		while (curr_piece != NULL)
		{	
			printf("[%s,", curr_piece->content);
			if (curr_piece->quote == Q_NONE)
				printf(" Q_NONE]");
			else if (curr_piece->quote == Q_SINGLE)
				printf(" Q_SINGLE]");
			else if (curr_piece->quote == Q_DOUBLE)
				printf(" Q_DOUBLE]");
			curr_piece = curr_piece->next;
		}
		curr_token = curr_token->next;
		printf(" / ");
	}
	printf("\n");
}

void	print_pieces_of_token(t_token *token)
{
	t_token	*curr;
	char	*str;

	curr = token;
	while (curr != NULL)
	{
		str = piece_to_str(curr->piece);
		printf("[%s]", str);
		free (str);
		curr = curr->next;
	}
	printf("\n");
}

void	print_commands(t_command *commands)
{
	int	i;
	t_redirect	*tmp;

	while (commands != NULL)
	{
		tmp = commands->redirs;
		i = 0;
		printf("[");
		while (commands->argv != NULL && commands->argv[i] != NULL)
		{
			printf("%s,", commands->argv[i]);
			i++;
		}
		printf("]");
		while (tmp != NULL)
		{
			printf(" -> [");
			printf("%s,", get_redir_symbol(tmp->type));
			printf("%s,", tmp->target);
			printf("quote : %d]", tmp->flag_quote);
			tmp = tmp->next;
		}
		commands = commands->next;
	}
	printf("\n");
}

void	print_debugger(t_token *tokens, t_command *commands)
{
	print_token(tokens);
	print_pieces_of_token(tokens);
	print_commands(commands);
	printf("%d\n", count_argv(tokens));
}