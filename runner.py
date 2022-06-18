import subprocess
import os
from sys import argv
from itertools import product

USAGE = ('Usage: <target> <path/to/DS-dir> <path/to/output-file> '
         '<epsilon> <delta> <cnt_threads>')


def read_args(argv):
    if (len(argv) != 7):
        print(USAGE)
        exit(0)

    return {
        'target': argv[1],
        'DS-dir': argv[2],
        'output': argv[3],
        'eps': argv[4],
        'del': argv[5],
        'threads': argv[6]
    }


def run_once(run_config):
    return subprocess.run([
        run_config['target'],
        run_config['ds'],
        run_config['eps'],
        run_config['del'],
        run_config['approximation'],
        run_config['distribution'],
        run_config['threads'],
        'none',
        run_config['format'],
    ], stdout=subprocess.PIPE).stdout.decode('utf-8')


def put_header_to_file(file, header):
    f = open(file, 'w')
    f.write(header)
    f.close()


def check_file_correct(config):
    header = subprocess.run([
        config['target'],
        'header'
    ], stdout=subprocess.PIPE).stdout.decode('utf-8')
    try:
        f = open(config['output'], 'r')
        probably_header = f.read(len(header))
        f.close()
        if probably_header != header:
            print('Header problems, file would be rewritten')
            put_header_to_file(config['output'], header)
    except FileNotFoundError:
        print('No output file, it will be created')
        put_header_to_file(config['output'], header)


def run_basis(config):
    ds_list = os.listdir(config['DS-dir'])
    config['format'] = 'csv'
    approximations = ['weak', 'strong']
    distributions = ['uniform', 'frequent', 'area-based', 'squared-frequency']
    for ds, approximation, distribution in product(ds_list, approximations, distributions):
        config['ds'] = os.path.join(config['DS-dir'], ds)
        config['approximation'] = approximation
        config['distribution'] = distribution
        f = open(config['output'], 'a')
        f.write(run_once(config))
        f.close()


if __name__ == '__main__':
    config = read_args(argv)
    check_file_correct(config)
    results = run_basis(config)
