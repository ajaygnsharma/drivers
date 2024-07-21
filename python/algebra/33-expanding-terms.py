#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from sympy import *

x,y = symbols ('x,y');

Fxy = (4 + x)*(2-y);

expd_expr = Fxy.expand();
display(expd_expr);

for i in range(0,3):
    res = expd_expr.subs({x:i, y:i}); 
    display(res);



