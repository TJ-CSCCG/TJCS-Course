# -*- coding: utf-8 -*-
"""
Created on Wed Mar 18 19:58:53 2020

@author: Aober
"""

import numpy as np
from sympy import Matrix

# generate ascii list of breathtaking
string = "breathtaking"
sstring = "rupotentoifv"
asciiLststr = []
asciiLstsstr = []
for char in range(0, len(string)):
    asciiLstsstr.append(ord(string[char]) - ord('a'))
    asciiLststr.append(ord(sstring[char]) - ord('a'))
    
# find divisor
divisor = []
for num in range(2, len(string)):
    if len(string) % num == 0:
        divisor.append(num)
        
    
def reflush(C, size, threshold):
    C = np.random.randint(
            -threshold,
            threshold,
            (size, size),
            dtype=int
        )
    return C
                

# hill
def hill(size, totalLen, inp, outp):
    try_time = totalLen // size
    
    C = np.zeros((size, size), dtype=int)
    C -= 100
    while True:
        flag = True
        for index in range(0, try_time):
            y = np.array(outp[index * size : (index + 1) * size], dtype=int)
            x = np.array(inp[index * size : (index + 1) * size], dtype=int)

            if ~(np.dot(x, C) % 26 == y).all():
                flag = False
                #print(y, x, C, np.dot(x, C) % 26)
                break
            
        if flag == True:
            return C
        else:
            C = reflush(C, size, 100)


for el in divisor:
    print(hill(el, len(string), asciiLststr, asciiLstsstr))
            
'''

a = np.array([1,2])
b = np.array([[1,2],[3,4]])
c = np.array([7, 10])
print(b[3])

'''

