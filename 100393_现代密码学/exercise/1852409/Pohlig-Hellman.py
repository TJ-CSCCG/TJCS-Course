# -*- coding: utf-8 -*-
"""
Created on Sun Jun  7 00:33:19 2020

@author: Aober
"""

def EX_GCD(a,b,arr): #扩展欧几里得
    if b == 0:
        arr[0] = 1
        arr[1] = 0
        return a
    g = EX_GCD(b, a % b, arr)
    t = arr[0]
    arr[0] = arr[1]
    arr[1] = t - int(a / b) * arr[1]
    return g

def ModReverse(a,n): #ax=1(mod n) 求a模n的乘法逆x
    arr = [0,1,]
    gcd = EX_GCD(a,n,arr)
    if gcd == 1:
        return (arr[0] % n + n) % n
    else:
        return -1


def PH(n, alpha, beta, q, c):
    iterNum = 0
    arr = []
    alen = 0
    beta_j = beta
    while iterNum <= c - 1:
        delta = beta_j**(n // (q**(iterNum + 1))) % n
        i = 0
        while delta != alpha**((i * n // q) % n) % n:
            print(alpha**((i * n // q) % n) % n, ' == ',
                    alpha,"**((", i, "*", n, "//", q, ") %", n, ") % ", n,' != ', delta)
            i = i + 1
        arr.append(i)
        alen += 1
        beta_j = beta_j * (ModReverse(alpha, n) ** (arr[alen-1] * (q ** iterNum) % n) % n) % n
        iterNum = iterNum + 1
    return arr

res1 = PH(28703, 5, 8563, 28703, 100)
res2 = PH(31153, 10, 12611, 31153, 100)

print(res1, res2)