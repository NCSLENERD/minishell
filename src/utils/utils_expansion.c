/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_expansion.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/04 12:00:00 by nmayela           #+#    #+#             */
/*   Updated: 2026/09/04 12:00:00 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../minishell.h"

int	is_expandable(char *content, int i)
{
	if (content[i] == '$')
	{
		if (content[i + 1] == '?')
			return (1);
		if (ft_isalpha(content[i + 1]) || content[i + 1] == '_')
			return (1);
	}
	return (0);
}

char	*expand_append(char *s1, char *s2)
{
	char	*res;

	if (!s2 && s1)
		return (s1);
	if (!s1 || !s2)
	{
		free(s1);
		return (NULL);
	}
	res = ft_strjoin(s1, s2);
	if (!res)
	{
		free(s1);
		return (NULL);
	}
	free(s1);
	return (res);
}

int	expand_piece(t_piece *piece, t_shell *shell)
{
	char	*new;

	if (piece->quote == Q_SINGLE)
		return (0);
	new = expand_str(piece->content, shell);
	if (!new)
		return (ERR_MALLOC);
	free(piece->content);
	piece->content = new;
	return (0);
}

int	expand_tokens(t_token *tokens, t_shell *shell)
{
	t_piece	*curr;

	while (tokens)
	{
		curr = tokens->piece;
		if (tokens->redir_type == R_HEREDOC)
			tokens = tokens->next;
		else if (tokens->type == MOT)
		{
			while (curr)
			{
				if (expand_piece(curr, shell) == ERR_MALLOC)
					return (ERR_MALLOC);
				curr = curr->next;
			}
		}
		tokens = tokens->next;
	}
	return (0);
}
