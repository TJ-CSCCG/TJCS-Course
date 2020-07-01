# -*- coding: utf-8 -*-
"""
Created on Thu Jun 11 22:44:16 2020

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
    
def fakeELG(alpha, beta, gama, delta, x, p, h, i, j):
    lamb = gama**h * alpha**i * beta**j % p
    miu = delta*lamb*ModReverse((h*gama-j*delta)%(p-1), p-1) % (p-1)
    x_ = lamb * (h * x + i * delta) * ModReverse((h*gama-j*delta)%(p-1), p-1) % (p-1)
    
    print(lamb, miu, x_)
    if beta**lamb*lamb**miu%p == alpha**x_%p:
        print('s')
        print(beta**lamb*lamb**miu%p, alpha**x_%p)
    else:
        print(beta**lamb*lamb**miu%p, alpha**x_%p)
    
fakeELG(2,132,29,51,100,467,102,45,293)
    
    