import os
import sys

if __name__ == "__main__":
    args = sys.argv

    if (len(args) == 1):
        print("error: no TTF font input.")
        print("usage: python3 ttf2header.py <TTF font>")
        exit(0)

    if (not os.path.exists(args[1])):
        print("error: failed to open TTF font.")
        exit(0)

    font_name = os.path.splitext(args[1])[0]
    font_name = font_name.replace('.', '')
    font_name = font_name.replace('/', '')
    font_name = font_name.replace('\\', '')
    array_name = font_name.replace('-', '')
    array_name = array_name.replace('_', '')

    array_data = ""
    with open(args[1], 'rb') as font_file:
        data = font_file.read().hex()
        for i in range(0, len(data), 2):
            array_data += "0x{}{}, ".format(data[i], data[i + 1])

    with open(f"{font_name}.h", 'w') as header_file:
        header_file.write(
f"""// Created from {font_name}.ttf, by ttf2header.py
#pragma once
#include <cstdint>

uint8_t {array_name}TTF[] = {{ {array_data} }};
""")