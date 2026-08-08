from src.parsing.tokenizer import tokenizer

def parse_simple(tokens):
    simple_cmd = {
        "type": "command",
        "cmd": None,
        "args": [],
        "stdin": None,
        "stdout": None,
        "background": False
    }

    if tokens and tokens[-1] == "&":
        simple_cmd["background"] = True
        tokens = tokens[:-1]

    if tokens[0] in ['>', '>>', '<'] and len(tokens) > 2:
        i = 2
        while (tokens[i] in ['>', ">>", '<']):
            i += 2
        simple_cmd["cmd"] = tokens[i]
        i = 0
    else:
        simple_cmd["cmd"] = tokens[0]
        i = 1

    while i < len(tokens):
        token = tokens[i]

        if token == ">":
            filename = tokens[i+1]
            simple_cmd["stdout"] = {"file": filename, "mode": "w"}
            i += 2
            continue

        if token == ">>":
            filename = tokens[i+1]
            simple_cmd["stdout"] = {"file": filename, "mode": "a"}
            i += 2
            continue

        if token == "<":
            filename = tokens[i+1]
            simple_cmd["stdin"] = {"file": filename}
            i += 2
            continue

        if token == simple_cmd["cmd"]:
            i += 1
            continue

        simple_cmd["args"].append(token)
        i += 1
    return simple_cmd

def split_pipe(tokens):
    segments = []
    current = []

    for token in tokens:
        if token == "|":
            segments.append(current)
            current = []
        else:
            current.append(token)
    if current:
        segments.append(current)
    return segments

def parse_pipeline(tokens):
    pipeline_cmd = {
        "type": "pipeline",
        "commands": [],
        "background": False
    }

    if tokens and tokens[-1] == "&":
        pipeline_cmd["background"] = True
        tokens = tokens[:-1]

    splited_tokens = split_pipe(tokens)
    for segment in splited_tokens:
        pipeline_cmd["commands"].append(parse_simple(segment))

    return pipeline_cmd


def parse(tokens):

    if tokens and tokens[0] in ["for", "if"]:
        return parse_simple(tokens)

    if "|" in tokens:
        return parse_pipeline(tokens)
    else:
        return parse_simple(tokens)

# line = "> b >> c echo hey"
# tokens = tokenizer(line)
# print(parse_simple(tokens))


