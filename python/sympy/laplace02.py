import sympy
sympy.init_printing(use_unicode=True, use_latex=True);

import matplotlib.pyplot as plt
from IPython.display import display, Latex

# First we need all symbols named short
exp=sympy.exp
sin=sympy.sin
cos=sympy.cos
cosh=sympy.cosh
sinh=sympy.sinh

def L(f):
    return sympy.laplace_transform(f, t, s, noconds=True);

t,s,n = sympy.symbols('t, s, n');
a = sympy.symbols('a', real=True, positive=True);


func = [a, 
        1, 
        4,
        t,
        exp(-a*t),
        exp(a*t),
        t*exp(-a*t),
        sin(a*t),
        cos(a*t),
        sinh(a*t),
        cosh(a*t),
        t**n]

for f in func:
    Fs = L(f);
    dp_str = f"${sympy.latex(f)} : {sympy.latex(Fs)}$"
    display(Latex(dp_str));

