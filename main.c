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

int	process_line(char *line, t_shell *shell)
{
	t_token	*tokens;
	t_command	*commands;
	int	ret;

	tokens = NULL;
	commands = NULL;
	ret = lexer(line, &tokens);
	if (ret == ERR_SYNTAX)
	{
		shell->exit_code = 2;
		return (ERR_SYNTAX);
	}
	else if (ret == ERR_MALLOC)
		return (ERR_MALLOC);
	if (check_syntax(tokens) == ERR_SYNTAX)
	{
		shell->exit_code = 2;
		free_tokens(&tokens);
		return (ERR_SYNTAX);
	}
	if (parser(tokens, &commands) == ERR_MALLOC)
	{
		free_tokens(&tokens);
		return (ERR_MALLOC);
	}
 	print_token(tokens);
	print_pieces_of_token(tokens);
	print_commands(commands);
	printf("%d\n", count_argv(tokens));
	free_commands(&commands);
	free_tokens(&tokens);
	return (0);
}

int	main(int argc, char **argv, char **envp)
{
	char *line;
	t_shell	shell;

	(void)argc;
	(void)argv;
	(void)envp;
	shell.env = NULL;
	shell.exit_code = 0;
	while(1)
	{
		line = readline("minishell$ ");
		if (line == NULL)
		{
			printf("exit\n");
			break;
		}
		if (line[0] != '\0')
			add_history(line);
		if (process_line(line, &shell) == ERR_MALLOC)
		{
			free(line);
			exit(1);
		}
		free(line);
	}
	return (shell.exit_code);
}