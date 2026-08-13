/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 19:31:20 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/05 19:31:22 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "minishell.h"

int	main()
{
	char *line;

	while(1)
	{
		line = readline("minishell$ ");
		if(line == NULL)
		{
			printf("exit\n");
			break;
		}
		add_history(line);
		//lexer(line);
		free(line);
	}
}