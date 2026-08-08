import sys
from src.parsing.parser import parse

def builtin_for(cmd_struct):
    args = cmd_struct["args"]
    var_name = args[0]
    target_var = f"${var_name}"

    if len(args) < 4:
        print("Erreur syntaxe: for <var> in ... do ... done", file=sys.stderr)
        return False

    if args[1] != "in":
        print(f"Erreur syntaxe: attendu 'in', reçu '{args[1]}'", file=sys.stderr)
        return False

    try:
        index_do = args.index("do")
        index_done = args.index("done")
    except ValueError:
        print("Erreur syntaxe: 'do' ou 'done' manquant", file=sys.stderr)
        return False

    values = [token for token in args[2:index_do] if token !=";"]
    body = args[index_do+1:index_done]

    from src.execution.executor_simple import exec_simple
    from src.execution.exec_pipeline import exec_pipe
    from src.utils.handle_builtins import handle_builtin

    if body and body[0] == ";":
        body.pop(0)
    if body and body[-1] == ";":
        body.pop()

    if not body:
        return True

    for val in values:
        current_tokens = []
        for token in body:
            new_token = token.replace(target_var, val)
            current_tokens.append(new_token)

        parsed_commands = parse(current_tokens)

        if isinstance(parsed_commands, dict):
            parsed_commands = [parsed_commands]

        for cmd in parsed_commands:
            if cmd["type"] == "command":
                exec_simple(cmd)
            else:
                exec_pipe(cmd)
    return True