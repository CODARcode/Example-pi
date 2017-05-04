#!/usr/bin/env python3

import sys
import subprocess
import os.path
import time


def run_pi(method, precision, iterations):
    p = subprocess.run(['./pi-gmp', method, str(precision), str(iterations)],
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                       check=True)
    return p.stdout.strip()


def check_pi_digits(result):
    ref_path = os.path.join(os.path.dirname(__file__), 'pi1M.txt')
    with open(ref_path, 'rb') as ref_file:
        ref = ref_file.read().strip()
        if ref[0] != result[0]:
            return 0
        if result[1] != ord('.'):
            return 1
        for i in range(2, min(len(ref), len(result))):
            if ref[i] != result[i]:
                break
        # don't count decimal
        return i-1


def main():
    start = time.time()
    result = run_pi(sys.argv[1], sys.argv[2], sys.argv[3])
    print(str(result, 'ascii')[:79])
    dt = time.time() - start
    print('correct digits:', check_pi_digits(result))
    print('time', dt)


if __name__ == '__main__':
    main()
