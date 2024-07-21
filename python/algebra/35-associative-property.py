#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from sympy import *

w,x,y,z = symbols('w,x,y,z');

x = w *(4-w) + (1/w**2)*(1+w);

F1 = x*(y+z);
F2 = (3/x)+(x**2);

f1f2= F1 * F2;

display("multiplied f1*f2=");

display(f1f2);
display(f1f2.simplify());

comm_prop = (F1*F2) - (F2*F1);
if(comm_prop == 0):
    display("Expression is Commutative");
    




