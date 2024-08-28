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
    (".-.-.-", "."),
    ("--..--", ","),
    ("---...", ":"),
    ("..--..", "?"),
    (".----.", "'"),
    ("-....-", "-"),
    ("-..-.", "/"),
    ("-.--.", "("),
    ("-.--.-", ")"),
    ("-...-", "="),
]


def ensure_table_length(table: list, index: int):
    while index >= len(table):
        table.append(1)


TABLE = []

# build the tree
for code, letter in INPUTS:
    index = 0
    for element in code:
        ensure_table_length(TABLE, index)
        if element == ".":  # dit
            index = index * 2 + 1
        else:
            index = index * 2 + 2
    ensure_table_length(TABLE, index)
    TABLE[index] = ord(letter)
    # current.letter = letter

print("TABLE = [")
offset = 0
while offset < len(TABLE):
    line = ", ".join([f"0x{v:02x}" for v in TABLE[offset : offset + 10]])
    print(f"      {line},")
    offset += 10
print("]")
