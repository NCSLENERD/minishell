#include "../../minishell.h"

/*
** expand_str : remplace $VAR et $? dans content, renvoie une chaine allouee.
** get_env_value vit desormais dans exec/path.c (signature t_env *, char *).
*/
char	*expand_str(char *content, t_shell *shell)
{
    int i;
    int j;
    char    *acc;
    char    *stock;
    int len;

    acc = ft_strdup("");
    i = 0;
    // "a$USER b$?"
    while (content[i])
    {
        if (content[i] == '$')
        {
            if (content[i + 1] == '?')
            {
                convert = ft_itoa(shell->exit_code);
                ft_strjoin(acc, stock);
                free(stock);
            }
            else if (ft_isalpha(content[i + 1]) || content[i + 1] == '_')
            {
                len = i;
                while (ft_isalpha(content[i + 1]) || content[i + 1] == '_')
                    i++;
                len  = i - len;
            }
        }
        i++;
    }  
	return (NULL);
}
s