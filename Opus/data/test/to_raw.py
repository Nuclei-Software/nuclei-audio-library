#!/usr/bin/env python3

import sys
import re
import pathlib
import numpy as np

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: %s <log.txt>" % (sys.argv[0]))
        sys.exit(1)

    logfile = pathlib.Path(sys.argv[1])
    if not logfile.exists():
        print("File not found: %s" % (logfile))

    with open(logfile, 'r') as f:
        data = []
        find_start = False
        for line in f:
            check_start = re.match(r"^\[", line)
            if not find_start:
                if check_start:
                    find_start = True
                continue
            
            check_stop = re.match(r"^\]", line)
            if check_stop:
                break

            numbers = line.split()
            for num in numbers:
                data.append(int(num, 16))

        np.array(data, dtype=np.uint8).tofile(logfile.stem + ".raw")