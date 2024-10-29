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

    pattern = r"^CSV, ima_adpcm_(en|de)code, (\d+)$"
    base_enc = []
    base_dec = []
    with open(base, 'r') as f:
        base_lines = f.readlines()
        for line in base_lines:
            result = re.match(pattern, line)
            if result:
                if result.group(1) == "en":
                    base_enc.append(int(result.group(2)))
                elif result.group(1) == "de":
                    base_dec.append(int(result.group(2)))
    
    new_enc = []
    new_dec = []
    with open(new, 'r') as f:
        new_lines = f.readlines()
        for line in new_lines:
            result = re.match(pattern, line)
            if result:
                if result.group(1) == "en":
                    new_enc.append(int(result.group(2)))
                elif result.group(1) == "de":
                    new_dec.append(int(result.group(2)))

    if len(base_enc) != len(new_enc) or len(base_dec) != len(new_dec):
        print("Length mismatch!")
        sys.exit(1)

    # discard the last data, because it's not accurate
    base_enc_avg = np.mean(base_enc[:-1])
    base_dec_avg = np.mean(base_dec[:-1])
    new_enc_avg = np.mean(new_enc[:-1])
    new_dec_avg = np.mean(new_dec[:-1])

    print("base_enc_avg = %.2f" % (base_enc_avg))
    print("base_dec_avg = %.2f" % (base_dec_avg))
    print("new_enc_avg = %.2f" % (new_enc_avg))
    print("new_dec_avg = %.2f" % (new_dec_avg))

    print("speedup ratio enc = %.2f" % (base_enc_avg / new_enc_avg))
    print("speedup ratio dec = %.2f" % (base_dec_avg / new_dec_avg))