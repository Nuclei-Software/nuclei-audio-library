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

    pattern = r"^CSV, speex_(echo_cancellation|preprocess_run), (\d+)$"
    base_aec = []
    base_preprocess = []
    with open(base, 'r') as f:
        base_lines = f.readlines()
        for line in base_lines:
            result = re.match(pattern, line)
            if result:
                if result.group(1) == "echo_cancellation":
                    base_aec.append(int(result.group(2)))
                elif result.group(1) == "preprocess_run":
                    base_preprocess.append(int(result.group(2)))
    
    new_aec = []
    new_preprocess = []
    with open(new, 'r') as f:
        new_lines = f.readlines()
        for line in new_lines:
            result = re.match(pattern, line)
            if result:
                if result.group(1) == "echo_cancellation":
                    new_aec.append(int(result.group(2)))
                elif result.group(1) == "preprocess_run":
                    new_preprocess.append(int(result.group(2)))

    if len(base_aec) != len(new_aec) or len(base_preprocess) != len(new_preprocess):
        print("Length mismatch!")
        sys.exit(1)

    base_aec_avg = np.mean(base_aec)
    base_preprocess_avg = np.mean(base_preprocess)
    new_aec_avg = np.mean(new_aec)
    new_preprocess_avg = np.mean(new_preprocess)

    print("base_aec_avg = %.2f" % (base_aec_avg))
    print("base_preprocess_avg = %.2f" % (base_preprocess_avg))
    print("new_aec_avg = %.2f" % (new_aec_avg))
    print("new_preprocess_avg = %.2f" % (new_preprocess_avg))

    print("speedup ratio AEC = %.2f" % (base_aec_avg / new_aec_avg))
    print("speedup ratio preprocess = %.2f" % (base_preprocess_avg / new_preprocess_avg))