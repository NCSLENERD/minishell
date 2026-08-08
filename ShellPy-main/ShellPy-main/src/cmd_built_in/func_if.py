import os
import sys
from src.parsing.parser import parse


def condition_check(condition_type, condition1, condition2=None):

    #tests sur les repertoires
    if condition_type == "-e":              #si le repertoire existe
        return os.path.exists(condition1)
    elif condition_type == "-d":            #si le repertoire est un dossier
        return os.path.isdir(condition1)
    elif condition_type == "-f":            #si le repertoire est un fichier
        return os.path.isfile(condition1)
    elif condition_type == "-r":            #si le fichier est lisible
        return os.access(condition1, os.R_OK)
    elif condition_type == "-w":            #si le fichier est ecrivable
        return os.access(condition1, os.W_OK)
    elif condition_type == "-x":            #si le fichier est executable
        return os.access(condition1, os.X_OK)

    #tests sur les chaines
    elif condition_type == "-z":
        return len(str(condition1)) == 0
    elif condition_type == "-n":
        return len(str(condition1)) > 0
    elif condition_type == "=":
        return str(condition1) == str(condition2)
    elif condition_type == "!=":
        return str(condition1) != str(condition2)

    #tests numeriques
    elif condition_type in ["-eq", "-ne", "-gt", "-lt", "-ge", "-le"]:
        try:
            v1 = int(condition1)
            v2 = int(condition2)
        except ValueError:
            print(f"Erreur syntaxe: entier attendu pour {condition_type}", file=sys.stderr)
            return False

        if condition_type == "-eq":
            return v1 == v2
        elif condition_type == "-ne":
            return v1 != v2
        elif condition_type == "-gt":
            return v1 > v2
        elif condition_type == "-lt":
            return v1 < v2
        elif condition_type == "-ge":
            return v1 >= v2
        elif condition_type == "-le":
            return v1 <= v2

    elif condition_type == "!":
        return not condition1
    elif condition_type == "-a":
        return condition1 and condition2
    elif condition_type == "-o":
        return condition1 or condition2


def builtin_if(cmd_struct):

    args = cmd_struct["args"]
    try:
        index_then = args.index("then")
        index_fi = args.index("fi")
    except ValueError:
        print("Erreur syntaxe: 'then' ou 'fi' manquant", file=sys.stderr)
        return False

    index_else = None
    if "else" in args:
        index_else = args.index("else")

    cond = [x for x in args[:index_then] if x!=";"]
    if not cond:
        print("Erreur: aucune condition fournie", file=sys.stderr)
        return False

    is_true = False

    if len(cond) == 3:
        is_true = condition_check(cond[1], cond[0], cond[2])
    elif len(cond) == 2 and cond[0].startswith("-"):
        is_true = condition_check(cond[0], cond[1])
    else:
        if cond[0].lower() == "true": is_true = True
        elif cond[0].lower() == "false": is_true = False
        else:
            print(f"Erreur: condition mal formée : {cond}", file=sys.stderr)
            return False

    tokens_to_exec = []

    if is_true:
        limit = index_else if index_else is not None else index_fi
        tokens_to_exec = args[index_then + 1 : limit]
    else:
        if index_else:
            tokens_to_exec = args[index_else + 1 : index_fi]

    if tokens_to_exec and tokens_to_exec[0] == ";": tokens_to_exec.pop(0)
    if tokens_to_exec and tokens_to_exec[-1] == ";": tokens_to_exec.pop()

    if not tokens_to_exec:
        return True

    from src.execution.executor_simple import exec_simple
    from src.execution.exec_pipeline import exec_pipe
    from src.utils.handle_builtins import handle_builtin

    parse_if = parse(tokens_to_exec)

    if isinstance(parse_if, dict):
        parse_if = [parse_if]

    for cmd in parse_if:
        if cmd["type"] == "command":
            exec_simple(cmd)
        else:
            exec_pipe(cmd)

    return True