# -*- coding: utf-8 -*-

from sympy import *
from IPython.display import Math
import numpy as num

x, y = symbols('x,y');

Fxy = (4+x)*(2-y);
display(Fxy.expand());

mat = num.zeros([3,3]);
#display(sympify(mat));

for _x in range(0,3):
    for _y in range(0,3):
        mat[_x,_y] = Fxy.subs({x:_x, y:_y});


display(sympify(mat));
