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

int	lexer(char *line, t_token **head)
{
	int	i;
	int	ret;

	*head = NULL;
	i = 0;
	while (line[i] != '\0')
	{
		while(is_space(line[i]))
			i++;
		if	(line[i] == '\0')
			break;
		if (is_operator(line[i]))
			ret = read_op(line, &i, head);
		else
			ret = read_word(line, &i, head);
		if (ret != 0)
		{
			free_tokens(head);
			return (ret);
		}
	}
	return (0);
}
