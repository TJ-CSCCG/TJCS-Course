# -*- coding: utf-8 -*-
"""
Created on Mon Jun 15 19:28:57 2020

@author: Aober
"""

import random
def fermat(test, iterNum):
    
    for i in range(0, iterNum):
        rand = random.randint(1, test-1)
        if rand**(test-1) % test != 1:
            return False
    else:
        return True
    
for i in range(3, 100):
    print(i, "is", fermat(i, i*4))