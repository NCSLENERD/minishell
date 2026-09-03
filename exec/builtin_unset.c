#include "../minishell.h"

int builtin_unset(t_command *cmd, t_shell *shell)
{
    int i;
    int ret;

    ret = 0;
    i = 1;
    while(cmd->argv[i])
    {
        if (!is_valid_identifier(cmd->argv[i]) || ft_strchr(cmd->argv[i], '=') != NULL)
            ret = unset_error(cmd->argv[i]);
        else
            remove_env_key(&shell->env, cmd->argv[i]);
        i++;
    }
    return(ret);
}

void remove_env_key(t_env **env, char *key)
{
    t_env   *curr;
    t_env   *prev;

    prev = NULL;
    curr = *env;
    while (curr)
    {
        if (ft_strncmp(curr->key, key,(ft_strlen(key) + 1)) == 0)
        {
            if(prev == NULL)
            {
                *env = curr->next;
                free_curr(curr);
                return;
            }
            else
            {
                prev->next = curr->next;
                free_curr(curr);
                return;
            }
        }
        prev = curr;
        curr = curr->next;
    }
    return;
}

void    free_curr(t_env *curr)
{
    free(curr->key);
    free(curr->value);
    free(curr);
}

int unset_error(char *arg)
{
    ft_putstr_fd("minishell: unset: `", 2);
    ft_putstr_fd(arg, 2);
    ft_putstr_fd("': not a valid identifier\n", 2);
    return (1);
}