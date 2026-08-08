import os
from src.utils.shell_state import get_shell_state, is_shell_state
from src.utils.users_vars import get_users_vars, is_user_var

def expand_var(tokens):
    new_tokens = []
    for token in tokens:
        if token.startswith("$"):
            var_name = token[1:]

            if is_shell_state(var_name):
                value = get_shell_state(var_name)
                new_tokens.append(value)
                continue
            if is_user_var(var_name):
                value = get_users_vars(var_name)
                new_tokens.append(value)
                continue

            value = os.environ.get(var_name, "")
            new_tokens.append(value) # pas de gestion d'erreur comme un vrai shell (ex: echo n'affiche rien)
        else:
            new_tokens.append(token)
    return new_tokens

