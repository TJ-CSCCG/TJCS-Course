# -*- coding: utf-8 -*-
"""
Created on Thu Jun 11 19:13:38 2020

for i in range(0, 1103):
    if i ** 5 % 1103 == 896:
        print(i)
        break
    else:
        pass

@author: Aober
"""

'''
没做出来
'''
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

def L1bit(beta, p):
    return 0 if beta**((p-1) // 2) % p else 1


def OracleDiscLog(p, alpha, beta, L2beta):
    lst = []
    x = L1bit(beta, p)
    alpha_rev = ModReverse(alpha, p)
    print("alpha reverse", alpha_rev)
    lst.append(x)
    beta = beta * (alpha_rev ** x % p) % p
    i = 1
    while beta != 1:
        x_i = L2beta
        gama = beta**((p + 1) // 4) % p
        if L1bit(gama, p) == x_i:
            beta = gama
        else:
            beta = p - gama
        print(beta, "beta")
        lst.append(x_i)
        beta = beta * (alpha_rev ** x_i % p) % p
        print(beta, "beta")
        i = i + 1
        
    return lst

def L2vali(beta, p, alpha):
    valibeta = beta ** ((p + 1) // 4) % p
    for i in range(0, p, 2):
        #print(i)
        if alpha ** (i // 2) % p == valibeta:
            return 1,i//2
        elif alpha ** (i // 2) % p == p - valibeta:
            return -1,i//2
        else:
            pass
    return 0


'''
print(L2vali(25219, 1103, 5))
print(L2vali(841, 1103, 5))
print(L2vali(163, 1103, 5))
print(L2vali(532, 1103, 5))
print(L2vali(625, 1103, 5))
print(L2vali(656, 1103, 5))
'''
print(OracleDiscLog(1103, 5, 896, 1))