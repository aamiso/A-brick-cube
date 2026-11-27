#!/usr/bin/env python3
import struct
import sys

src, dst, symbol = sys.argv[1:4]
data = open(src, "rb").read()
data += b"\0" * ((-len(data)) % 4)
words = struct.unpack("<" + "I" * (len(data) // 4), data)
with open(dst, "w", encoding="ascii") as f:
    f.write("#pragma once\n#include <3ds.h>\n")
    f.write("static const u32 " + symbol + "[] = {\n")
    for i in range(0, len(words), 8):
        f.write("    " + ", ".join("0x%08Xu" % x for x in words[i:i+8]) + ",\n")
    f.write("};\n")
    f.write("static const u32 " + symbol + "_size = sizeof(" + symbol + ");\n")
