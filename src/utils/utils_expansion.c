#include "../../minishell.h"

char *get_env_value(t_shell *shell, char *str)
{
    t_env *curr;
    int i;

    curr = shell->env;
    while (curr)
    {
        i = 0;
        while (curr->key[i] && str[i] && curr->key[i] == str[i])
            i++;
        if (curr->key[i] == '\0' && str[i] == '\0')
            return (curr->value);
        curr = curr->next;
    }
    return (NULL);
}

char  *expand_str(char *content, t_shell *shell)
{
    
}