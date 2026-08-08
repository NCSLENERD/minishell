from src.utils.shell_state import shell_state

user_vars = {
}

def is_user_var(var_name):
    return var_name in user_vars

def get_users_vars(var_name):
    return str(user_vars[var_name])

def is_affect(cmd):
    if "=" in cmd["cmd"] and not cmd["args"]:
            return 1
    return 0



