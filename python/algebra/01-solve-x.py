# -*- coding: utf-8 -*-
from sympy import *

x = symbols('x');

expr = 2*x + 4 - 9;
display(expr);
display(solve(expr));



expr = x**2 - 4;
display(expr);
display(solve(expr));

expr = x**2 - x;
display(expr);
display(solve(expr));
 