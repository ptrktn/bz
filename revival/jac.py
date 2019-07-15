#!/usr/bin/python

import sys

def main(fname):
    
    vars = ("fkinet", "rkinet", "kf", "kr")
    nvars = 19
    nx = 17
    
    print("from sympy import *")
    
    for i in vars:
        for j in range(1, nvars + 1):
            print("%s%d = Symbol(\"%s%d\")" % (i, j, i, j))
            
    i = "xdot"
    for j in range(1, nx + 1):
        print("%s%d = Function(\"%s%d\")" % (i, j, i, j))
        print("x%d = Symbol(\"x%d\")" % (j, j))

        
    f = open(fname)
    while True:
        line = f.readline().rstrip()
        if not line:
            break
        print(line)
    f.close()

    print("print(\"# -*-octave-*-\\n\")")
    print("print(\" 1; \\n\")")
    print("print(\"function j = jac (x, t)\")")
    print("print(\"    global kf kr ;\")")
    print("print(\"    j = zeros(17, 17) ;\\n\")")
    
    for i in range(1, nx + 1):
        for j in range(1, nx + 1):
            print("print(\"    # xdot(%d) = \")," % i)
            print("print(xdot%d)" % i)
            print("print(\"    j(%d, %d) = \")," % (i, j))
            print("print(diff(xdot%d, x%d))," % (i, j))
            print("print(\" ; \\n\")")


    print("print(\"endfunction\")")
        
        
if "__main__" == __name__:
    main(sys.argv[1])

   
