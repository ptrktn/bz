#!/usr/bin/python

import sys

def main(fname=None):
    
    vars = ("fkinet", "rkinet", "kf", "kr")
    nvars = 19
    nx = 17
    
    print("from sympy import *")
    
    for i in vars:
        for j in range(1, nvars + 1):
            print("%s%d = Symbol(\"%s%d\")" % (i, j, i, j))
            
    i = "xdot"
    for j in range(1, nx + 1):
        #exec("%s%d = Function(\"%s%d\")" % (i, j, i, j))
        print("%s%d = Function(\"%s%d\")" % (i, j, i, j))
        print("x%d = Symbol(\"x%d\")" % (j, j))

        
    #try:
    f = open(fname)
    while True:
        line = f.readline().rstrip()
        if not line:
            break
        print(line)
        #exec(line)
        
    #except:
    #    pass
    #finally:
    f.close()

    print("print(\" 1; \")")
    print("print(\"function j = jac (x, t)\")")
    print("print(\"    global kf kr ;\")")
    print("print(\"    j(17, 17) = zeros(17, 17);\")")
    
    for i in range(1, nx + 1):
        for j in range(1, nx + 1):
            print("print(\"    j(%d, %d) = \")," % (i, j))
            #%s ; " % diff(xdot1, x17))
            #print("j(%d, %d) = ", )
            print("print(diff(xdot%d, x%d))," % (i, j))
            print("print(\" ; \")")


    print("print(\"endfunction\")")
        
        
if "__main__" == __name__:
    main(sys.argv[1])
    #main()

   
