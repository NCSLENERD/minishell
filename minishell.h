/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nmayela <nmayela@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 19:55:05 by nmayela           #+#    #+#             */
/*   Updated: 2026/08/05 19:55:59 by nmayela          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

typedef struct s_token
{
    e_type *type;
    s_piece *piece;
} t_token;

typedef enum e_type
  {
      Q_NONE,
      Q_SINGLE,
      Q_DOUBLE
  }   t_type;

typedef struct s_piece
{
    char *content;
    e_quote quote;
    s_piece *next;
} t_piece;

typedef enum e_quote
  {
      Q_NONE,
      Q_SINGLE,
      Q_DOUBLE
  }   t_quote;

typedef struct s_noeud
{
    char    *sep;
    s_noeud *left;
    s_noeud *right;
} t_noeud;

typedef struct s_command
{

}t_command;