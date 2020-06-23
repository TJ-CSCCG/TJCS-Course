# -*- coding: utf-8 -*-
"""
Created on Sun Jun 14 16:34:10 2020

@author: Aober
"""

def lianfen(nume,deno,iterNum):
    cnt = 2
    a = [nume//deno, deno//(nume-deno*(nume//deno))]
    p = [a[0], a[1]*a[0]+1]
    q = [1, a[1]]
    yume = [(nume-deno*(nume//deno)), deno - a[1] * (nume-deno*(nume//deno))]
    while cnt < iterNum - 1:
        a.append(yume[0] // yume[1])
        p.append(a[cnt] * p[cnt - 1] + p[cnt - 2])
        q.append(a[cnt] * q[cnt - 1] + q[cnt - 2])
        yume = [yume[1], yume[0] - a[cnt] * yume[1]]
        #print(cnt, yume, a,p[cnt - 1], p[cnt - 2],p, q)
        if p[cnt] == nume and q[cnt] == deno:
            break
        cnt = cnt + 1
    
    return {"a": a, "p": p, "q": q}

def seq(n, alst, b, tlst):
    dictarr = []
    for a, t in zip(alst, tlst):
        if (a * b - 1) % t == 0:
            phi_n = (a * b - 1) // t
            dictarr.append({"+":n - phi_n + 1,"*":n})
    return dictarr
           
def solve(plus, time):
    import math
    print((plus**2 - 4 * time), plus, time)
    delta = math.sqrt(plus**2 - 4 * time) if plus**2 - 4 * time > 0 else 0
    return ((plus + delta) // 2, (plus - delta) // 2)

def lianfensep(n, b, iterNum=100):
    lf = lianfen(n, b, iterNum)
    print(lf)
    plustime = seq(n, lf['p'], b, lf['q']) if n > b else seq(n, lf['q'], b, lf['p']) 
    print(plustime)
    for dic in plustime:
        tup = solve(dic['+'], dic['*'])
        print(tup)
        
lianfensep(317940011, 77537081)