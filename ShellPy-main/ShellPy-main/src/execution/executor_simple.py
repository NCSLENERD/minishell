from src.utils.background import background_processes
from src.utils.handle_builtins import handle_builtin
from src.cmd_built_in.cd import cd
import subprocess
from src.execution.redir import handle_redir
from src.utils.shell_state import shell_state
import os
import sys
from src.utils.users_vars import is_affect, user_vars
from src.parsing.expand_var import expand_var


def exec_simple(cmd_simple):
    built_in = ["cd", "if", "for", "exit", "unset", "help"]

    if is_affect(cmd_simple):
        var, val = cmd_simple["cmd"].split("=", 1)
        user_vars[var] = val
        shell_state["?"] = 0
        return 1

    stdin, stdout = handle_redir(cmd_simple)
    if stdin == -1:
        shell_state["?"] = -1
        return -1

    commande = [cmd_simple["cmd"]] + cmd_simple["args"]

    if cmd_simple["cmd"] in built_in:
        return handle_builtin(cmd_simple)

    else:
        try:
            if cmd_simple["background"]:
                proc = subprocess.Popen(commande, stdin=stdin, stdout=stdout)
                background_processes.append(proc)
                print(f"[{proc.pid}] {' '.join(commande)} is running in background")
            else:
                proc = subprocess.Popen(commande, stdin=stdin, stdout=stdout)
                proc.wait()
                shell_state["?"] = proc.returncode
        except FileNotFoundError:
            print(f"Erreur : commande '{cmd_simple['cmd']}' introuvable", file=sys.stderr)
        finally:
            if stdin:
                stdin.close()
            if stdout:
                stdout.close()
