#!/usr/bin/env python

import sys
import string
from sympy import *
import re
import os

rctns = list(())
x = list(())
excess = list(())
xdot = list(())
rspcs = []
initial = {}
constant = {}
simulation = {}
kf = {}
kr = {}
t_interval = 10
t_points = 10

class R:
    def __init__(self):
        self.text = None
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

        self.text = " ".join(" ".join(r.split()).split(" "))
        
        [self.lhs, self.rhs] = r.split(cs)

        for c in r.replace(cs, "+").replace("*", "+").split("+"):
            c = c.strip()
            for d in c.split():
                if not(d in constant):
                    self.s[d] = 1 + self.s.get(d, 0)

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
                if re.search(r'^EXCESS\s', line):
                    if len(line.split()) > 1:
                        for a in line.split()[1:]:
                            if not a in excess:
                                excess.append(a)
                    continue
                elif re.search(r'^INITIAL\s', line):
                    if len(line.split()) > 2:
                        a, val = line.split()[1:3]
                        initial[a] = val
                    continue
                elif re.search(r'^CONSTANT\s', line):
                    if len(line.split()) > 2:
                        a, val = line.split()[1:3]
                        constant[a] = val
                    continue
                elif re.search(r'^RATES\s', line):
                    vals = line.split()
                    if 3 == len(vals):
                        a = int(vals[1])
                        kf[a] = vals[2]
                    if 3 < len(vals):
                        a = int(vals[1])
                        kf[a] = vals[2]
                        kr[a] = vals[3]
                    continue
                elif re.search(r'^SIMULATION\s', line):
                    vals = line.split()
                    if len(vals) > 2:
                        global t_interval, t_points
                        if "T_INTERVAL" == vals[1]:
                            t_interval = int(vals[2])
                        elif "T_POINTS" == vals[1]:
                            t_points = int(vals[2])
                        elif "MAXIMUM_STEP_SIZE" == vals[1]:
                            simulation["MAXIMUM_STEP_SIZE"] = vals[2]
                    continue

                n += 1
                r.reaction(line, nr)
                rctns.append(r)
                r = R()
                nr += 1


def proc_rspcs():
    for r in rctns:
        for s in r.species():
            i = s.strip()
            if not(i in rspcs):
                rspcs.append(i)


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
        if s in excess or s in constant:
            continue
        x.append(s)

    
    for s in rspcs:
        
        if s in excess or s in constant:
            continue
        
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
    e = df

    print(" IN: %s" % e)
    for s in rspcs:
        s1 = "__%s__" % s
        
        if s in excess or s in constant:
            s2 = " %s " % s
        else:
            i = 1 + x.index(s)
            s2 = " x(%i) " % i
            
        e = e.replace(s1, s2)
        
    e = re.sub(r'__(k[f,r])(\d+)__', r' \1(\2) ', e)

    print("OUT: %s" % e)
    
    return e

def lsoda_output(fbase):
    fname = "%s.h" % fbase
    mname = "%s.mat" % fbase
    n = len(rctns)
    neq = len(xdot)
    
    try:
        fp = open(fname, "w")

        fp.write("\n/*\n")
        
        sx = 0
        for r in rctns:
            if len(r.text) > sx:
                sx = len(r.text)

        sx += 3
        fmt = "# %%-%ds (R%%d)\n" % sx
        for r in rctns:
            fp.write(fmt % (r.text, r.i))

        fp.write("\n*/\n")

        defs="""
#define x(i) (x[i-1])
#define xdot(i) (xdot[i-1])
#define kf(i) kf[i]
#define kr(i) kr[i]
"""
        
        fp.write("%s\n" % defs)

        
        fp.write("#define T_END %d\n" % t_interval)
        fp.write("#define T_DELTA (1/ (double) %d)\n" % t_points)
        fp.write("#define NEQ %d\n\n" % neq)

        fp.write("\nstatic void erhelper_init(double *x, double *abstol, double *reltol)\n{\n")
        fp.write("\n/* initial conditions */\nx[0] = 0;\n")

        for a in x:
            if a in initial:
                i = 1 + x.index(a)
                fp.write("/* %s */\nx[%d] = %s ;\n" % (a, i, initial[a]))        

        i = 0
        while i <= neq:
            fp.write("abstol[%d] = 1.0E-7;\n" % i)
            fp.write("reltol[%d] = 1.0E-7;\n" % i)
            i += 1
	
        fp.write("\n}\n")
                
        fp.write("\nstatic void fex(double t, double *x, double *xdot, void *data)\n{\n")

        fp.write("double kf[NEQ], kr[NEQ+1];")
        
        if len(excess):
            fp.write("\n/* Species in excess %s */\n" % " ".join(excess))
            for a in excess:
                if initial.has_key(a):
                    fp.write("#define %s (%s)\n" % (a, initial[a]))

        if len(constant):
            fp.write("\n/* Constants %s */\n" % " ".join(constant))
            for a in constant:
                fp.write("#define %s (%s)\n" % (a, constant[a]))
            
        fp.write("\n/* forward reaction rates */\n")
        for a in kf:
            fp.write("kf[%d] = %s ;\n" % (a, kf[a]))
        fp.write("\n/* reverse reaction rates */\n")
        for a in kr:
            fp.write("kr[%d] = %s ;\n" % (a, kr[a])) 

        # FIXME zeros
        i = 0
        for dx in xdot:
            fp.write("    xdot(%d) = %s ; \n" % (i + 1, xdot[i]))
            i += 1

        fp.write("\n}\n")

    except:
        raise
    finally:
        fp.close()

    try:
        os.chmod(fname, 0644)
    except:
        pass


def octave_output(fbase):
    fname = "%s.m" % fbase
    mname = "%s.mat" % fbase
    n = len(rctns)
    
    try:
        fp = open(fname, "w")

        fp.write("#!/usr/bin/octave -qf\n")
        fp.write("# -*-octave-*-\n")

        sx = 0
        for r in rctns:
            if len(r.text) > sx:
                sx = len(r.text)

        sx += 3
        fmt = "# %%-%ds (R%%d)\n" % sx
        for r in rctns:
            fp.write(fmt % (r.text, r.i))

        fp.write("more off\n")
        fp.write("global kf kr ;\n")

        if len(excess):
            fp.write("\n# Species in excess\nglobal %s ;\n" % " ".join(excess))
            for a in excess:
                if initial.has_key(a):
                    fp.write("%s = %s ;\n" % (a, initial[a]))

        if len(constant):
            fp.write("\n# Constants\nglobal %s ;\n" % " ".join(constant))
            for a in constant:
                fp.write("%s = %s ;\n" % (a, constant[a]))
            
        fp.write("\n# forward reaction rates\nkf = zeros(%d, 1) ;\n" % n)
        for a in kf:
            fp.write("kf(%d) = %s ;\n" % (a, kf[a])) 
        fp.write("\n# reverse reaction rates\nkr = zeros(%d, 1) ;\n" % n)
        for a in kr:
            fp.write("kr(%d) = %s ;\n" % (a, kr[a])) 
        fp.write("\n# initial conditions\nx0 = zeros(%d, 1) ;\n" % len(xdot))
        for a in x:
            if a in initial:
                i = 1 + x.index(a)
                fp.write("# %s\nx0(%d) = %s ;\n" % (a, i, initial[a]))
        fp.write("\nfunction xdot = f (x, t)\n")
        fp.write("    global kf kr ;\n")

        if len(excess):
            fp.write("    global %s ;\n" % " ".join(excess))
        if len(constant):
            fp.write("    global %s ;\n" % " ".join(constant))

        fp.write("    xdot = zeros(%d, 1) ;\n" % len(xdot))
        i = 0
        for dx in xdot:
            fp.write("    xdot(%d) = %s ; \n" % (i + 1, xdot[i]))
            i += 1
        
        fp.write("endfunction\n")

        fp.write("\nlsode_options(\"integration method\", \"stiff\") ;\n")

        if "MAXIMUM_STEP_SIZE" in simulation:
            fp.write("lsode_options(\"maximum step size\", %s) ;\n" %
                     simulation["MAXIMUM_STEP_SIZE"])

        fp.write("lsode_options(\"absolute tolerance\", 1e-7) ;\n")
        fp.write("lsode_options(\"relative tolerance\", 1e-7) ;\n")
        fp.write("\nt_interval = %d ;\n" % t_interval)
        fp.write("t_points = %d ;\n" % t_points)
        fp.write("t = linspace(0, t_interval, t_interval * t_points) ;\n")
        fp.write("tstart = cputime ;\n")
        fp.write("  [y, istate, msg] = lsode (\"f\", x0, t) ;\n")
        fp.write("lsode_options\n")
        fp.write("istate\n")
        fp.write("msg\n")
        fp.write("printf('Total CPU time: %f seconds\\n', cputime - tstart) ;\n")
        fp.write("if istate != 2\n    exit(1) ;\nendif\n")
        fp.write("mat = [rot90(t, -1), y] ;\n")
        fp.write("save %s mat ;\n" % mname)
  
    except:
        raise
    finally:
        fp.close()

    try:
        os.chmod(fname, 0755)
    except:
        pass

def validate_input():
    for a in excess:
        if a not in initial:
            raise Exception("Missing initial value for: %s" % a)
        
def main(fname):

    read_r(fname)

    validate_input()
    
    for r in rctns:
        print(r.i)
        print(r.lhs)
        print(r.rhs)
        print(r.rates)
        print(r.kinet())
        print(r.kinet(False))
        print(r.species())
        
    proc_rspcs()

    proc_r()

    i = 0
    for dx in xdot:
        print("xdot(%d) = %s ; " % (i + 1, xdot[i]))
        i += 1

    octave_output("erhelper")
    lsoda_output("erhelper")
    
    print("X %s" % x)
    print("EXCESS %s" % excess)
    print("INITIAL %s" % initial)
    print("CONSTANT %s" % constant)


if "__main__" == __name__:
    main(sys.argv[1])

   
