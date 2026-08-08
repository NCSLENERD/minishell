import sys

from src.utils.shell_state import shell_state
from src.utils.users_vars import user_vars

def unset(var):
    if var in user_vars:
        del user_vars[var]
        shell_state["?"] = 0
        return 1
    print(f"La variable '{var}' n'existe pas.", file=sys.stderr)
    shell_state["?"] = 1
    return 0