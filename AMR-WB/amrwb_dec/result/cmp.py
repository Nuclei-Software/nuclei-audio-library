#!/usr/bin/env python3

import sys
import pathlib
import re
import numpy as np

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: %s <base.csv> <new.csv>" % (sys.argv[0]))
        sys.exit(1)

    base = pathlib.Path(sys.argv[1])
    if not base.exists():
        print("File not found: %s" % (base))
    new = pathlib.Path(sys.argv[2])
    if not new.exists():
        print("File not found: %s" % (new))

    pattern = r"^CSV, decode, (\d+)$"
    base_data = []
    with open(base, 'r') as f:
        base_lines = f.readlines()
        for line in base_lines:
            result = re.match(pattern, line)
            if result:
                base_data.append(int(result.group(1)))
    
    new_data = []
    with open(new, 'r') as f:
        new_lines = f.readlines()
        for line in new_lines:
            result = re.match(pattern, line)
            if result:
                new_data.append(int(result.group(1)))

    if len(base_data) != len(new_data):
        print("Length mismatch: %d != %d" % (len(base_data), len(new_data)))
        sys.exit(1)

    base_avg = np.mean(base_data)
    new_avg = np.mean(new_data)
    print("base_avg = %.2f" % (base_avg))
    print("new_avg = %.2f" % (new_avg))

    print("speedup ratio = %.2f" % (base_avg / new_avg))