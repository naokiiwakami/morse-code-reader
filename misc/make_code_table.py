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

TABLE = []
num_entries = ord("Z") - ord("/") + 1
for i in range(num_entries * 2):
    TABLE.append(0)


class Element:
    def __init__(self, character=None):
        self.character = character
        self.left = None
        self.right = None


CURRENT_PLACE_HOLDER = ord("9") + 1


def fill_table(node, index, TABLE):
    global CURRENT_PLACE_HOLDER
    if node is None:
        return
    if node.left is not None:
        if node.left.character is None:
            if CURRENT_PLACE_HOLDER >= ord("A"):
                raise RuntimeError("out of placeholders")
            node.left.character = chr(CURRENT_PLACE_HOLDER)
            CURRENT_PLACE_HOLDER += 1
        left = ord(node.left.character) - ord("0") + 1
        fill_table(node.left, left, TABLE)
    else:
        left = 0
    if node.right is not None:
        if node.right.character is None:
            if CURRENT_PLACE_HOLDER >= ord("A"):
                raise RuntimeError("out of placeholders")
            node.right.character = chr(CURRENT_PLACE_HOLDER)
            CURRENT_PLACE_HOLDER += 1
        right = ord(node.right.character) - ord("0") + 1
        fill_table(node.right, right, TABLE)
    else:
        right = 0
    TABLE[index * 2] = left * 2
    TABLE[index * 2 + 1] = right * 2


root = Element("/")

# build the tree
for code, character in INPUTS:
    current = root
    for element in code:
        if element == ".":
            if current.left is None:
                current.left = Element()
            current = current.left
        else:
            if current.right is None:
                current.right = Element()
            current = current.right
    current.character = character

# traverse
fill_table(root, 0, TABLE)
TABLE[0] |= 0x80
TABLE[1] |= 0x80
for i in range(22, (ord("A") - ord("/")) * 2):
    TABLE[i] |= 0x80

for i, value in enumerate(TABLE):
    print(f"{i:02x} {value:02x} : {chr(int(i / 2) - 1 + ord('0'))}")

print("TABLE = [")
offset = 0
while offset < len(TABLE):
    line = ", ".join([f"0x{v:02x}" for v in TABLE[offset : offset + 10]])
    print(f"      {line},")
    offset += 10
print("]")
