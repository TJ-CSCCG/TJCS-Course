# -*- coding: utf-8 -*-
"""
Created on Sat Jun  6 21:58:16 2020

@author: Aober
"""

def gcd(a, b):
    return gcd(b, a % b) if b != 0 else a

def fn(x, n):
    return (x*x + 1) % n

def sep(n):
    x2i = 2
    cursor = 2
    arr = [1,2]
    gcd_ = 1
    while gcd_ == 1:
        arr.append(fn(x2i, n))
        x2i = fn(x2i, n)
        cursor += 1
        if cursor % 2 == 0:
            gcd_ = gcd(arr[(cursor-1) // 2] - arr[cursor-1], n)
    if gcd(arr[(cursor-1) // 2], arr[cursor-1]) == n:
        return -1
    else:
        print(cursor, x2i, gcd(arr[(cursor-1) // 2] - arr[cursor-1], n))
        print(arr[(cursor-1) // 2], arr[cursor-1])

sep(262063)
sep(9420457)
sep(181937053)
sep(28703)