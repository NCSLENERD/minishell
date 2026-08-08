import os

shell_state = {
    "?": 0, #last exit code (cmd exit + int change cette valeur)
    "$": os.getpid(), #pid du processus du shell
    "0": "ShellPy" #nom du shell
}

def is_shell_state(var_name):
    return var_name in shell_state

def get_shell_state(var_name):
    return str(shell_state[var_name])