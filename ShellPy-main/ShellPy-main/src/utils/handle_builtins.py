import sys
from src.cmd_built_in.cd import cd
from src.cmd_built_in.unset import unset
from src.utils.shell_state import shell_state
from src.cmd_built_in.func_if import builtin_if
from src.cmd_built_in.func_for import builtin_for

def handle_builtin(cmd):

    if cmd["type"] != "command":
        return False

    if not cmd["cmd"]:
        return False

    if cmd["cmd"] == "cd":
        arg = cmd["args"][0] if cmd["args"] else None
        code = cd(arg)
        shell_state["?"] = code
        return True

    if cmd["cmd"] == "if":
        return builtin_if(cmd)

    if cmd["cmd"] == "for":
        return builtin_for(cmd)

    if cmd["cmd"] == "exit":
        code = int(cmd["args"][0]) if cmd["args"] else 0
        shell_state["?"] = code
        sys.exit(code)

    if cmd["cmd"] == "unset":
        arg = cmd["args"][0] if cmd["args"] else None
        unset(arg)
        return True

    if cmd["cmd"] == "help":
        help()

    return False