#!/usr/bin/env python3
"""
Cheetah post-processing script to analyze results of running pi-gmp with
different parameters.
"""
import os.path
import json
import sys

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt


PI_DIGITS = None

def compare_pi_digits(result_path):
    """
    Compare string of pi digits in @result_path with actual digits from
    reference file (stored in global). Both are assumed to be strings of
    decimal digits including the decimal point.

    Return (correct_digits, incorrect_digits)
    """
    with open(result_path) as result_file:
        result = result_file.read().strip()
        if PI_DIGITS[0] != result[0]:
            return 0, len(result)
        if result[1] != '.':
            return 1, len(result)-1
        for i in range(2, min(len(PI_DIGITS), len(result))):
            if PI_DIGITS[i] != result[i]:
                break
        # don't count decimal
        return i-1, len(result) - i


def get_args():
    parser = argparse.ArgumentParser(description='plot pi computation results')
    parser.add_argument('-s', '--stdout-name', required=True,
                    help='Name of file containing the output pi digits')
    parser.add_argument('-p', '--param-name', required=True,
                    help='Name of file containing the parameters as JSON')
    parser.add_argument('-w', '--walltime-name', required=True,
                    help='Name of file containing the walltime')
    parser.add_argument('directories', nargs='+',
                    help='Path to directory containing stdout and param files')
    return parser.parse_args()


def load_pi_reference():
    global PI_DIGITS
    ref_path = os.path.join(os.path.dirname(__file__), '..', 'pi1M.txt')
    with open(ref_path) as ref_file:
        PI_DIGITS = ref_file.read().strip()


def main():
    params_json_path = sys.argv[1]
    with open(params_json_path) as f:
        runs = json.load(f)
    outf = open('analysis.txt', 'w')
    print('method, iterations, precision, max_digits', file=outf)
    load_pi_reference()
    df = None
    nruns = len(runs)
    # Read results into a pandas DataFrame
    for i, params in enumerate(runs):
        dir_path = params['output_directory']
        # TODO: should this be part of the JSON?
        stdout_path = os.path.join(dir_path, 'codar.cheetah.run-output.txt')
        walltime_path = os.path.join(dir_path, 'codar.cheetah.walltime.txt')
        digits, waste_digits = compare_pi_digits(stdout_path)
        with open(walltime_path) as f:
            walltime = int(f.read().strip())
        values = list(params.values()) + [digits, waste_digits, walltime]
        if df is None:
            keys = list(params.keys()) + ['correct_digits', 'waste_digits',
                                          'walltime']
            df = pd.DataFrame(index=np.arange(0, nruns), columns=keys)
        df.loc[i] = values

    # Analyize the results. In particular, for each number of
    # iterations, determine the minimum precision that captures all
    # digits.
    fig, axs = plt.subplots(4)
    for i, (method, df_method) in enumerate(df.groupby('method')):
        iterations_groupby = df_method.groupby('iterations')
        df2_method = pd.DataFrame(index=np.arange(0, len(iterations_groupby)),
                                  columns=keys)
        for j, (iterations, df_method_iter) in enumerate(iterations_groupby):
            max_digits = np.max(df_method_iter['correct_digits'])
            has_max_digits = df_method_iter['correct_digits'] == max_digits
            df_max_digits = df_method_iter[has_max_digits]
            #print(df_max_digits)
            precision = np.min(df_max_digits['precision'])
            print(method, iterations, precision, max_digits, file=outf)
            df2_method.loc[j] = df_max_digits[
                        df_max_digits['precision']==precision].iloc[0]
        #print(df2_method)

        df_method.plot.scatter(x='iterations', y='correct_digits', ax=axs[i],
                       label=method, marker='o', logx=True)
    # TODO: have convention to save to special subdir?
    plt.savefig('analysis.png')
    outf.close()


if __name__ == '__main__':
    main()
