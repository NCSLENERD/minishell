import sys

def handle_redir(cmd):
    stdin = None
    stdout = None

    if cmd["stdout"]:
        stdout = open(cmd["stdout"]["file"], cmd["stdout"]["mode"])
    if cmd["stdin"]:
        try:
            stdin = open(cmd["stdin"]["file"], "r")
        except FileNotFoundError:
            print(f"Erreur : fichier '{cmd['stdin']['file']}' introuvable", file=sys.stderr)
            stdin = -1
        except PermissionError:
            print(f"Erreur : permission refusée pour le fichier '{cmd['stdin']['file']}'", file=sys.stderr)
            stdin = -1

    return stdin, stdout
