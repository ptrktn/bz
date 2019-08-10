#!/usr/bin/env python

import sys
import string
from sympy import *
import re

rctns = list(())
x = list(())
xdot = list(())
rspcs = {}

class R:
    def __init__(self):
        self.i = 0
        self.lhs = None
        self.rhs = None
        self.rates = [0, 0]
        self.k = ["kf", "kr"]
        self.s = {}

    def reaction(self, r, n):
        self.i = n

        if " <==> " in r:
            self.rates = [1, 1]
            cs = " <==> "
        elif " ==> " in r:
            self.rates = [1, 0]
            cs = " ==> "
        elif " <== " in r:
            self.rates = [0, 1]
            cs = " <== "
        else:
            raise Exception("Type not found: \" <==> \", \" ==> \" or \" <== \" (%d)" % n)

        [self.lhs, self.rhs] = r.split(cs)

        for c in r.replace(cs, "+").split("+"):
            c = c.strip()
            self.s[c] = 1 + self.s.get(c, 0)

    def kinet(self, d=True):
        if d:
            a = self.lhs
            j = 0
        else:
            a = self.rhs
            j = 1
            
        k = False
        
        if self.rates[j] > 0:
            k = self.k[j] + str(self.i) + " * " + " * ".join(a.split("+"))

        return k
    
    def species(self):
        return self.s

    def smc(self, d, y):
        c = 0
        
        if d:
            r = self.lhs
        else:
            r = self.rhs
        
        for i in map(string.strip, r.split("+")):
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
                r.reaction(line, nr)
                rctns.append(r)
                r = R()
                nr += 1


def proc_rspcs():
    for r in rctns:
        for s in r.species():
            i = s.strip()
            rspcs[i] = 1 + rspcs.get(i, 0)

            
def symbols(x):
    r = []

    for i in " ".join(x.split()).split(" "):
        i = i.strip()
        if i is None:
            pass
        elif i.isdigit() or "+" == i or "-" == i or "*" == i:
            r.append(i)
        else:
            r.append("Symbol('__%s__')" % i)
    return " ".join(r)


def proc_r():
    i = 1
    
    for s in rspcs:
        a = []
        for r in rctns:
            # A + B -> C + D
            k = r.kinet(True)
            y = r.smc(True, s)
            if y > 0 and k:
                a.append("- %d * %s" % (y, k))
            y = r.smc(False, s)
            if y > 0 and k:
                a.append("+ %d * %s" % (y, k))

            # A + B <- C + D
            k = r.kinet(False)
            y = r.smc(False, s)
            if y > 0 and k:
                a.append("- %d * %s" % (y, k))
            y = r.smc(True, s)
            if y > 0 and k:
                a.append("+ %d * %s" % (y, k))

        # print("d%s = %s" % (s, " ".join(a)))
        b = symbols(" ".join(a))
        exec("df = %s" % b)
        print(df)
        xdot.append(subst_x(str(df)))
        i += 1


def subst_x(df):
    i = 1
    e = df
    for s in rspcs:
        s1 = "__%s__" % s
        s2 = " x(%i) " % i
        e = e.replace(s1, s2)
        i += 1
        
    e = re.sub(r'__(k[f,r])(\d+)__', r' \1(\2) ', e)

    return e


def octave_output(fbase):
    try:
        fname = "%s.m" % fbase
        fname = "%s.m" % fbase 
        mname = "%s.mat" % fbase
        n = len(xdot)

        fp = open(fname, "w")

        fp.write("#!/usr/bin/env octave -qf\n")
        fp.write("# -*-octave-*-\n")
        fp.write("more off\n")
        fp.write("global kf kr ;\n")
        fp.write("# forward reaction rates\nkf = zeros(%d, 1) ;\n" % n)
        fp.write("# reverse reaction rates\nkr = zeros(%d, 1) ;\n" % n)
        fp.write("# initial conditions\nx0 = zeros(%d, 1) ;\n" % n)
        fp.write("\nfunction xdot = f (x, t)\n")
        fp.write("    global kf kr ;\n")
        fp.write("    xdot = zeros(%d, 1) ;\n" % n)
        i = 0
        for dx in xdot:
            fp.write("    xdot(%d) = %s ; \n" % (i + 1, xdot[i]))
            i += 1
        
        fp.write("endfunction\n\n")

        fp.write("t = linspace (0, 25, 25 * 100) ;\n")
        fp.write("tstart = cputime ;\n")
        fp.write("  [y, istate, msg] = lsode (\"f\", x0, t) ;\n")
        fp.write("  istate\n")
        fp.write("  msg\n")
        fp.write("printf('Total CPU time: %f seconds\\n', cputime - tstart) ;\n")
        fp.write("mat = [rot90(t, -1), y] ;\n")
        fp.write("save %s mat ;\n" % mname)
  
    except:
        raise
    finally:
        fp.close()

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
        # print("X %s" % s)
        x.append(s)

    proc_r()

    i = 0
    for dx in xdot:
        print("xdot(%d) = %s ; " % (i + 1, xdot[i]))
        i += 1

    octave_output("erhelper")
        
if "__main__" == __name__:
    main(sys.argv[1])

   
