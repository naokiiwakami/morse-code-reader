#!/usr/bin/env python3

INPUTS = [
    (".-", "A"),
    ("-...", "B"),
    ("-.-.", "C"),
    ("-..", "D"),
    (".", "E"),
    ("..-.", "F"),
    ("--.", "G"),
    ("....", "H"),
    ("..", "I"),
    (".---", "J"),
    ("-.-", "K"),
    (".-..", "L"),
    ("--", "M"),
    ("-.", "N"),
    ("---", "O"),
    (".--.", "P"),
    ("--.-", "Q"),
    (".-.", "R"),
    ("...", "S"),
    ("-", "T"),
    ("..-", "U"),
    ("...-", "V"),
    (".--", "W"),
    ("-..-", "X"),
    ("-.--", "Y"),
    ("--..", "Z"),
    (".----", "1"),
    ("..---", "2"),
    ("...--", "3"),
    ("....-", "4"),
    (".....", "5"),
    ("-....", "6"),
    ("--...", "7"),
    ("---..", "8"),
    ("----.", "9"),
    ("-----", "0"),
]


def ensure_table_length(table: list, index: int):
    while index >= len(table):
        table.append(0)


TABLE = []

# build the tree
for code, character in INPUTS:
    index = 0
    for element in code:
        ensure_table_length(TABLE, index)
        if element == ".":  # dit
            TABLE[index] |= 0x1
            index = index * 2 + 1
        else:
            TABLE[index] |= 0x2
            index = index * 2 + 2
    ensure_table_length(TABLE, index)
    char_code = (ord(character) - ord("/")) << 2
    TABLE[index] |= char_code
    # current.character = character

print("TABLE = [")
offset = 0
while offset < len(TABLE):
    line = ", ".join([f"0x{v:02x}" for v in TABLE[offset : offset + 10]])
    print(f"      {line},")
    offset += 10
print("]")
