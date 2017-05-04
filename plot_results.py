#!/usr/bin/env python3
"""
Script to plot results of running pi-gmp with different parameters. The results
must be in subdirectories with a JSON file describing the parameters and a file
containing the output of the run (the digits of pi).
"""
import os.path
import argparse
import json

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
        result = result_file.read()
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
    ref_path = os.path.join(os.path.dirname(__file__), 'pi1M.txt')
    with open(ref_path) as ref_file:
        PI_DIGITS = ref_file.read()


def main():
    args = get_args()
    load_pi_reference()
    df = None
    nruns = len(args.directories)
    # Read results into a pandas DataFrame
    for i, dir_path in enumerate(args.directories):
        param_path = os.path.join(dir_path, args.param_name)
        stdout_path = os.path.join(dir_path, args.stdout_name)
        walltime_path = os.path.join(dir_path, args.walltime_name)
        digits, waste_digits = compare_pi_digits(stdout_path)
        with open(param_path) as f:
            params = json.load(f)
        with open(walltime_path) as f:
            walltime = int(f.read().strip())
        values = list(params.values()) + [digits, waste_digits, walltime]
        if df is None:
            keys = list(params.keys()) + ['correct_digits', 'waste_digits',
                                          'walltime']
            print('keys=', keys)
            df = pd.DataFrame(index=np.arange(0, nruns), columns=keys)
        df.loc[i] = values

    # Analyize the results. In particular, for each number of
    # iterations, determine the minimum precision that captures all
    # digits.
    fig, axs = plt.subplots(4, 4)
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
            print(method, iterations, precision, max_digits)
            df2_method.loc[j] = df_max_digits[
                        df_max_digits['precision']==precision].iloc[0]
        #print(df2_method)

        df_method.plot.scatter(x='iterations', y='correct_digits', ax=axs[i,0],
                       label=method, marker='o', logx=True)
        df2_method.plot(x='iterations', y='precision', ax=axs[i,1],
                        label=method, marker='o', logx=True)
        df2_method.plot(x='iterations', y='walltime', ax=axs[i,2],
                        label=method, marker='o', logx=True)
        df_method.plot.scatter(x='precision', y='walltime', ax=axs[i,3],
                       label=method, marker='.',
                       logx=True)
    plt.show()


if __name__ == '__main__':
    main()
