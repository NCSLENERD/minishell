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