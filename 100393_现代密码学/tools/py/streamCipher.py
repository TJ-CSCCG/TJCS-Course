# -*- coding: utf-8 -*-
"""
Created on Wed Mar 18 19:19:04 2020

@author: Aober
"""

import numpy as np
import random

plainTxt = []

# generator
for index in range(0,1000):
    string = ''
    for jndex in range(0,4):
        string += str(random.randint(0, 1))
    if string not in plainTxt:
        plainTxt.append(string)
    if len(plainTxt) == 16:
        break

# from 0000 to 1111
plainTxt.sort()

cipherTxt = []
# cipher generator: 20 bits
# z_{i+4} = \sum_{i}^{i+3} z_j mod 2
'''
for plain in plainTxt:
    for index in range(0, 20):
        sumPlain =  int(plain[index]) + \
                    int(plain[index + 1]) + \
                    int(plain[index + 2]) + \
                    int(plain[index + 3])
        new = sumPlain % 2
        plain += str(new)
    cipherTxt.append(plain)
'''

# cipher generator: 20 bits
# z_{i+4} = (z_i + z_{i+3}) mod 2
for plain in plainTxt:
    for index in range(0, 20):
        sumPlain = int(plain[index]) + int(plain[index + 3])
        new = sumPlain % 2
        plain += str(new)
    cipherTxt.append(plain)
print(cipherTxt)