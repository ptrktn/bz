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
xdot_raw = list(())
rspcs = []
initial = {}
constant = {}
simulation = {}
kf = {}
kr = {}
lsode_atol = "1E-6"
lsode_rtol = "1E-6"

class R:
    def __init__(self):
        self.text = None
        self.i = 0
        self.reactants = None
        self.products = None
        self.rates = [0, 0]
        self.k = ["kf", "kr"]
        self.s = {}
        self.kf = 1
        self.kr = 1

    def reaction(self, r, n):
        self.i = n

        if " <==> " in r:
            self.rates = [1, 1]
            cs = " <==> "
        elif " ==> " in r:
            self.rates = [1, 0]
            cs = " ==> "
        else:
            raise Exception("Type not found: \" <==> \" or \" ==> \" (%d)" % n)

        self.text = " ".join(" ".join(r.split()).split(" "))
        
        if "|" in r:
            [rct, rts] = map(string.strip, r.split("|"))
            if 1 == len(rts.split()) and " ==> " == cs:
                self.kf = rts.split()[0]
            elif 2 == len(rts.split()) and " <==> " == cs:
                [self.kf, self.kr] = rts.split()
            else:
                raise Exception("Malformed rate input: \"%s\" (%d)" % (r, n))
        else:
            rct = r.strip()

        [self.reactants, self.products] = rct.split(cs)

        for c in rct.replace(cs, "+").replace("*", "+").split("+"):
            c = c.strip()
            for d in c.split():
                if not(d in constant):
                    self.s[d] = 1 + self.s.get(d, 0)

    def kinet(self, d=True):
        if d:
            a = self.reactants
            j = 0
        else:
            a = self.products
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
            r = self.reactants
        else:
            r = self.products
        
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
                elif re.search(r'^SIMULATION\s', line):
                    vals = line.split()
                    if len(vals) > 2:
                        if "T_END" == vals[1]:
                            simulation["T_END"] = vals[2]
                        elif "T_POINTS" == vals[1]:
                            simulation["T_POINTS"] = vals[2]
                        elif "MAXIMUM_STEP_SIZE" == vals[1]:
                            simulation["MAXIMUM_STEP_SIZE"] = vals[2]
                        elif "INITIAL_STEP_SIZE" == vals[1]:
                            simulation["INITIAL_STEP_SIZE"] = vals[2]
                        else:
                            raise Exception("Invalid SIMULATION argument: %s"
                                            % vals[1])
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


def symbols2(x):
    r = []

    for i in " ".join(x.split()).split(" "):
        i = i.strip()
        if i is None:
            pass
        elif i.isdigit() or "+" == i or "-" == i or "*" == i:
            r.append(i)
        else:
            r.append("__%s__" % i)

    return " ".join(r)

                
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
        xdot_raw.append(subst_x(str(symbols2(" ".join(a)))))
        i += 1


def subst_x(df):
    e = df

    # print(" IN: %s" % e)
    for s in rspcs:
        s1 = "__%s__" % s
        
        if s in excess or s in constant:
            s2 = " %s " % s
        else:
            i = 1 + x.index(s)
            s2 = " x(%i) " % i
            
        e = e.replace(s1, s2)
        
    e = re.sub(r'__(k[f,r])(\d+)__', r' \1(\2) ', e)

    # print("OUT: %s" % e)
    
    return e

def lsoda_c_output(fbase):
    fname = "%s.h" % fbase
    mname = "%s.mat" % fbase
    n = len(rctns)
    neq = len(xdot)
    
    try:
        fp = open(fname, "w")

        fp.write("#ifndef __ERHELPER_H__\n#define __ERHELPER_H__\n\n/*\n")
        
        sx = 0
        for r in rctns:
            if len(r.text) > sx:
                sx = len(r.text)

        sx += 3
        fmt = " %%-%ds (R%%d)\n" % sx
        for r in rctns:
            fp.write(fmt % (r.text, r.i))

        i = 0
        fp.write("\n")
        for v in x:
            fp.write(" x(%d) %s\n" % (1 + i, v))
            i += 1

        fp.write("\n Plot command:\n\n xplot.sh erhelper.mat '%s'"
                 % " ".join(x))
            
        fp.write("\n*/\n")

        defs="""
#define x(i) (x[i-1])
#define x0(i) (x[i])
#define xdot(i) (xdot[i-1])
#define kf(i) kf[i]
#define kr(i) kr[i]
"""
        
        fp.write("%s" % defs)

        if len(excess):
            fp.write("\n/* Species in excess %s */\n" % " ".join(excess))
            for a in excess:
                if initial.has_key(a):
                    fp.write("#define %s (%s)\n" % (a, initial[a]))

        if len(constant):
            fp.write("\n/* Constants %s */\n" % " ".join(constant))
            for a in constant:
                fp.write("#define %s (%s)\n" % (a, constant[a]))

        fp.write("\n#define NEQ %d\n" % neq)
        fp.write("#define N_REACTIONS %d\n" % n)
        fp.write("#define T_END %s\n" % simulation["T_END"])
        fp.write("#define T_DELTA (1/ (double) %s)\n" % simulation["T_POINTS"])
        fp.write("#define LSODE_ATOL %s\n" % lsode_atol)
        fp.write("#define LSODE_RTOL %s\n" % lsode_rtol)
        if "MAXIMUM_STEP_SIZE" in simulation:
            fp.write("#define LSODE_HMAX %s\n" %
                     simulation["MAXIMUM_STEP_SIZE"])
        if "INITIAL_STEP_SIZE" in simulation:
            fp.write("#define LSODE_H0 %s\n" %
                     simulation["INITIAL_STEP_SIZE"])

        fp.write("\nstatic double kf[N_REACTIONS+1], kr[N_REACTIONS+1];\n")
        
        fp.write("\nstatic void erhelper_init(double *x, double *abstol, double *reltol)\n{\n")

        i = 0
        while i <= neq:
            fp.write("\tabstol[%d] = LSODE_ATOL;\n" % i)
            fp.write("\treltol[%d] = LSODE_RTOL;\n" % i)
            fp.write("\tx0(%d) = 0;\n" % i)
            i += 1
        
        fp.write("\n\t/* initial conditions */\n")
        for a in x:
            if a in initial:
                i = 1 + x.index(a)
                fp.write("\t/* %s */\n\tx0(%d) = %s ;\n" % (a, i, initial[a]))

        fp.write("\n\t/* forward reaction rates */\n")
        for r in rctns:
            if r.rates[0]:
                fp.write("\tkf(%d) = %s ;\n" % (r.i, r.kf))
        fp.write("\n\t/* reverse reaction rates */\n")
        for r in rctns:
            if r.rates[1]:
                fp.write("\tkr(%d) = %s ;\n" % (r.i, r.kr))
                
        fp.write("\n}\n")
                
        fp.write("\nstatic void fex(double t, double *x, double *xdot, void *data)\n{\n")
            
        # FIXME zeros
        i = 0
        for dx in xdot:
            fp.write("    /* %s */\n" % x[i])
            fp.write("    xdot(%d) = %s ; \n" % (i + 1, xdot_raw[i]))
            i += 1

        fp.write("\n}\n")

        fp.write("\n#endif\n")

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
        for r in rctns:
            if r.rates[0]:
                fp.write("kf(%d) = %s ;\n" % (r.i, r.kf))

        fp.write("\n# reverse reaction rates\nkr = zeros(%d, 1) ;\n" % n)
        for r in rctns:
            if r.rates[1]:
                fp.write("kr(%d) = %s ;\n" % (r.i, r.kr))

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

        fp.write("lsode_options(\"absolute tolerance\", %s) ;\n" % lsode_atol)
        fp.write("lsode_options(\"relative tolerance\", %s) ;\n" % lsode_rtol)
        fp.write("\nt_end = %s ;\n" % simulation["T_END"])
        fp.write("t_points = %s ;\n" % simulation["T_POINTS"])
        fp.write("t = linspace(0, t_end, t_end * t_points) ;\n")
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

    for p in ["T_END", "T_POINTS"]:
        if p not in simulation:
            raise Exception("Missing SIMULATIONS parameter: %s" % p)


def main(fname):

    read_r(fname)

    validate_input()
    
    for r in []:
        print(r.i)
        print(r.reactants)
        print(r.products)
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
    lsoda_c_output("erhelper")
    
    print("X %s" % x)
    print("EXCESS %s" % excess)
    print("INITIAL %s" % initial)
    print("CONSTANT %s" % constant)


if "__main__" == __name__:
    main(sys.argv[1])

   
