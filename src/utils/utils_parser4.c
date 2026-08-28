#include "../../minishell.h"

int	split_word2(t_piece **fields, t_piece *curr, char **acc, int *started)
{
	if (curr->quote != Q_NONE)
	{
		*acc = expand_append(*acc ,curr->content);
		if (*acc == NULL)
			return (ERR_MALLOC);
		*started = 1;
	}
	else
	{
		if (split_piece(curr->content, fields, acc, started) == ERR_MALLOC)
			return (ERR_MALLOC);
	}
	return (0);
}

int	split_word(t_token *token, t_piece **fields)
{
	int	started;
	char	*acc;
	t_piece	*curr;

	acc = ft_strdup("");
	if (!acc)
		return (ERR_MALLOC);
	started = 0;
	curr = token->piece;
	while (curr)
	{
		if (split_word2(fields, curr, &acc, &started) == ERR_MALLOC)
			return (ERR_MALLOC);
		curr = curr->next;
	}
	if (started == 1)
		return (add_field(fields, acc));
	else
	{
		free(acc);
		return (0);
	}
}

int	count_fields(t_piece *fields)
{
	int	count;

	count = 0;
	while (fields)
	{
		fields = fields->next;
		count++;
	}
	return (count);
}

int	fields_to_argv(t_command *cmd, t_piece *fields)
{
	int	i;
	int	count;

	i = 0;
	count = count_fields(fields);
	cmd->argv = malloc(sizeof(char *) * (count + 1));
	if (!cmd->argv)
		return (ERR_MALLOC);
	while (fields)
	{
		cmd->argv[i] = fields->content;
		fields->content = NULL;
		fields = fields->next;
		i++;
	}
	cmd->argv[i] = NULL;
	return (0);
}