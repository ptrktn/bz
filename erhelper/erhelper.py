#!/usr/bin/env python

import sys
import string
from sympy import *

rctns = list(())
rspcs = {}

class R:
    def __init__(self):
        self.i = 0
        self.lhs = None
        self.rhs = None
        self.rates = [0, 0]
        self.s = {}

    def reaction(self, x, d=True):
        if d:
            self.lhs = x
        else:
            self.rhs = x

        for i in x.split("+"):
            i = i.strip()
            self.s[i] = 1 + self.s.get(i, 0)

    def setrates(self, x):
        self.rates = list((int(x.split()[0]), int(x.split()[1])))
    
    def kinet(self, d=True):
        if d:
            a = self.lhs
            j = 0
        else:
            a = self.rhs
            j = 1
            
        k = False
        
        if self.rates[j] > 0:
            k = "kf" + str(self.i) + " * " + " * ".join(a.split("+"))

        return k
    
    def species(self):
        return self.s

    def smc(self, d, y):
        c = 0
        
        if d:
            x = self.lhs
        else:
            x = self.rhs
        
        for i in map(string.strip, x.split("+")):
            if y == i:
                c += 1
                
        return c

    
def xsmc(r, x):
    c = 0
    for i in map(string.strip, r.split("+")):
        if x == i:
            c += 1
    return c


def read_r(fname):
    with open(fname) as f:
        n = 0
        nr = 1
        r = R()
        for l in f:
            line = l.strip()
            if len(line) > 0 and '#' != line[0]:
                n += 1
                if 1 == n % 3:
                    r.reaction(line, True)
                elif 2 == n % 3:
                    r.reaction(line, False)
                else:
                    r.i = nr
                    r.setrates(line)
                    rctns.append(r)
                    r = R()
                    nr += 1


def proc_rspcs():
    for r in rctns:
        for s in r.species():
            i = s.strip()
            rspcs[i] = 1 + rspcs.get(i, 0)


def proc_r():

    for s in rspcs:
        a = []
        for r in rctns:
            # A + B -> C + D
            k = r.kinet(True)
            x = r.smc(True, s)
            if x > 0 and k:
                a.append("- %d * %s" % (x, k))
            x = r.smc(False, s)
            if x > 0 and k:
                a.append("+ %d * %s" % (x, k))

            # A + B <- C + D
            k = r.kinet(False)
            x = r.smc(False, s)
            if x > 0 and k:
                a.append("- %d * %s" % (x, k))
            x = r.smc(True, s)
            if x > 0 and k:
                a.append("+ %d * %s" % (x, k))

        print("d%s = %s" % (s, " ".join(a)))

        
def main(fname):
    
    read_r(fname)
    
    for r in rctns:
        print(r.i)
        print(r.lhs)
        print(r.rhs)
        print(r.rates)
        print(r.kinet())
        print(r.kinet(False))
        print(r.species())
        
    proc_rspcs()

    for s in rspcs:
        print("X %s" % s)

    proc_r()
    
        
if "__main__" == __name__:
    main(sys.argv[1])

   
