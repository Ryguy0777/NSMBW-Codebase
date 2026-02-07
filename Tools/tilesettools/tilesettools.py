#!/usr/bin/env python3

# tilesettools.py
# User-interfacing script to perform decode/encode operations.

import argparse
import json
import traceback
from pathlib import Path

import anim, grass, profoverride, rand

MODULES = {
    'a': anim,
    'g': grass,
    'p': profoverride,
    'r': rand
}

def main():
    parser = argparse.ArgumentParser(description='Converts tileset extension files to a human-readable format and back.')
    parser.add_argument('operation', type=str, choices=['decode', 'encode'], help='Operation to be executed.')
    parser.add_argument('filetype', type=str, choices=MODULES.keys(), help='Type of file to be parsed (see script for accepted values).')
    parser.add_argument('infile', type=Path, help='Input file.')
    args = parser.parse_args()

    if not args.infile.is_file():
        print('Input file not found!')
        return

    if args.operation == 'decode':
        with open(args.infile, 'rb') as src, open(args.infile.with_suffix('.json'), 'w') as dest:
            try:
                json.dump(MODULES[args.filetype].decode(src), dest, indent=4)
            except Exception:
                traceback.print_exc()
    else:
        with open(args.infile) as src, open(args.infile.with_suffix('.bin'), 'wb') as dest:
            try:
                dest.write(MODULES[args.filetype].encode(json.load(src)))
            except Exception:
                traceback.print_exc()

main()
