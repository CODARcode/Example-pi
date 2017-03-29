#!/usr/bin/env python3

import sys
import os.path


def compare_pi_digits(ref_path, result_path):
    with open(ref_path) as ref_file, open(result_path) as result_file:
        ref = ref_file.read()
        result = result_file.read()
        if ref[0] != result[0]:
            return 0
        if result[1] != '.':
            return 1
        for i in range(2, min(len(ref), len(result))):
            if ref[i] != result[i]:
                break
        # don't count decimal
        return i-1


def main():
    ref_path = os.path.join(os.path.dirname(__file__), 'pi1M.txt')
    digits = compare_pi_digits(ref_path, sys.argv[1])
    print(digits)


if __name__ == '__main__':
    main()
