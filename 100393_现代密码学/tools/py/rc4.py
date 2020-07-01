# -*- coding: utf-8 -*-
"""
Created on Thu Jun 18 00:22:06 2020

@author: Aober
"""

class rc4:
    def __init__(self, inp):
        self.inp = inp
        self.inplen = len(inp)
        self.T = self.inp
        self.S = [i for i in range(0, 256)]
        self.keyStream = []
        cnt = 0
        
        while len(self.T) < 256:
            self.T.append(self.inp[cnt])
            cnt += 1
            if cnt == self.inplen:
                cnt = 0
    
    def encode(self, toEncode):
        j = 0
        for i in range(0, 256):
            j = (j + self.T[i] + self.S[i]) % 256
            self.S[i], self.S[j] = self.S[j], self.S[i]
        
        i = 0
        j = 0
        for i in range(len(toEncode) - 1, -1, -1):
            i = (i + 1) % 256
            j = (j + self.S[i]) % 256
            self.S[i], self.S[j] = self.S[j], self.S[i]
            t = (self.S[i] + self.S[j]) % 256
            self.keyStream.append(self.S[t])
    
    def xor(self, toEncode):
        cipher = toEncode
        for i in range(0, len(self.keyStream)):
            cipher[i] = toEncode[i] ^ self.keyStream[i]
        return cipher
            

rc = rc4([3,2,3,4,5,6,7,8])
rc.encode([1,4,5,10,1])
print(rc.keyStream)
print(rc.xor([1,4,5,10,1]))
