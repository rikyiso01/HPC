#!/usr/bin/env python3

from sys import stdin

print(f"code=r'''{stdin.read()}'''\nwith open('assignment-less-dumb.c', 'w') as f:\n  f.write(code)")