/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 19:50:20 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/11 15:43:11 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "minishell.h"

void	print_token(t_token *token)
{
	t_piece *curr;

	curr = token->piece;
	while (curr != NULL)
	{
		printf("[%d, %s, %d] ,",token->type,curr->content, curr->quote);
		curr = curr->next;
	}
}


/*int main()
{

}*/
