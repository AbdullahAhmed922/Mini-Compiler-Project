import re

def lexer(code):

    token_pattern = r'int|print|[a-zA-Z_]\w*|\d+|==|=|\+|-|\*|/|\(|\)|;'

    tokens = re.findall(token_pattern, code)

    return tokens