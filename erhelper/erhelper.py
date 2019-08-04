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

    def setlhs(self, x):
        self.lhs = x
        for i in x.split("+"):
            i = i.strip()
            self.s[i] = 1 + self.s.get(i, 0)

    def setrhs(self, x):
        self.rhs = x
        for i in x.split("+"):
            i = i.strip()
            self.s[i] = 1 + self.s.get(i, 0)

    def setrates(self, x):
        self.rates = list((int(x.split()[0]), int(x.split()[1])))
    
    def fkinet(self):
        k = False
        if self.rates[0] > 0:
            k = "kf" + str(self.i) + " * " + " * ".join(self.lhs.split("+"))
        return k
    
    def rkinet(self):
        k = False
        if self.rates[1] > 0:
            k = "kr" + str(self.i) + " * " + " * ".join(self.rhs.split("+"))
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
                    r.setlhs(line)
                elif 2 == n % 3:
                    r.setrhs(line)
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
            k = r.fkinet()
            x = r.smc(True, s)
            if x > 0 and k:
                a.append("- %d * %s" % (x, k))
            x = r.smc(False, s)
            if x > 0 and k:
                a.append("+ %d * %s" % (x, k))

            # A + B <- C + D
            k = r.rkinet()
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
        print(r.fkinet())
        print(r.rkinet())
        print(r.species())
        
    proc_rspcs()

    for s in rspcs:
        print("X %s" % s)

    proc_r()
    
        
if "__main__" == __name__:
    main(sys.argv[1])

   
