#!/usr/bin/env python3

import sys
import pathlib
import re
import numpy as np

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: %s <base.csv> <new.csv>" % (sys.argv[0]))
        sys.exit(1)

    base_file = pathlib.Path(sys.argv[1])
    if not base_file.exists():
        print("File not found: %s" % (base_file))
    new_file = pathlib.Path(sys.argv[2])
    if not new_file.exists():
        print("File not found: %s" % (new_file))

    pattern = r"^CSV, mp3_decode, (\d+)$"
    base = []
    with open(base_file, "r") as f:
        base_lines = f.readlines()
        for line in base_lines:
            result = re.match(pattern, line)
            if result:
                base.append(int(result.group(1)))

    new = []
    new_preprocess = []
    with open(new_file, "r") as f:
        new_lines = f.readlines()
        for line in new_lines:
            result = re.match(pattern, line)
            if result:
                new.append(int(result.group(1)))

    if len(base) != len(new):
        print("Length mismatch!")
        sys.exit(1)

    base_avg = np.mean(base)
    new_avg = np.mean(new)

    print("base_avg = %.2f" % (base_avg))
    print("new_avg = %.2f" % (new_avg))

    print("speedup ratio = %.2f" % (base_avg / new_avg))
