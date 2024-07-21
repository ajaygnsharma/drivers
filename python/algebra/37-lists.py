#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from sympy import *

init_printing(use_latex=True);
x = symbols('x');

e1 = 2*x + x*(4-6*x) + x;
e2 = -x*(2/x + 4/(x**2)) + (4+x)/(4*x) ;
e3 = (x+3)*(x-3)*x*(1/(9*x));

eqs = [e1, e2, e3];

for eq in eqs:
    display(eq);
    #print_latex('%s'%{\\Longleftrightarrow})
    display(eq.expand());


