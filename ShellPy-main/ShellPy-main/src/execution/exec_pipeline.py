import subprocess
import sys

from src.execution.redir import handle_redir
from src.execution.executor_simple import exec_simple
from src.utils.background import background_processes
from src.utils.shell_state import shell_state
from src.utils.users_vars import is_affect


def exec_pipe(pipe):
    processus = []
    prev_stdout = None
    stdout_pipe = None
    built_in = ["cd", "if", "for", "exit", "unset", "help"]

    if len(pipe["commands"]) ==  1:
        print("Erreur : une commande ne peut pas finir par un pipe", file=sys.stderr)
        return -1

    for i, cmd in enumerate(pipe["commands"]):
        if is_affect(cmd):
            continue #ignorer les affect de var comme bash

        stdin, stdout = handle_redir(cmd)
        if prev_stdout:
            stdin = prev_stdout

        if i < len(pipe["commands"]) - 1:
            stdout_pipe = subprocess.PIPE
        else:
            stdout_pipe = stdout

        if cmd["cmd"] in built_in:
            exec_simple(cmd)
        else:
            commande = [cmd["cmd"]] + cmd["args"]
            try:
                proc = subprocess.Popen(commande, stdin=stdin, stdout=stdout_pipe)
                processus.append(proc)
                prev_stdout = proc.stdout
            except FileNotFoundError:
                print(f"Commande introuvable : {cmd['cmd']}", file=sys.stderr)
            finally:
                if stdin:
                    stdin.close()
                if stdout:
                    stdout.close()

    if pipe["background"]:
        print(f"[{processus[0].pid}] pipeline is running in background") #bash recupere le premier pid du processus lancé
        background_processes.append(processus[0])
    else:
        for proc in processus:
            proc.wait()
            shell_state["?"] = proc.returncode


