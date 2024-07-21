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

def L(F):
    return sympy.inverse_laplace_transform(F, s, t);

t,s,n = sympy.symbols('t, s, n');
a = sympy.symbols('a', real=True, positive=True);


func = [1/(2*s - 3), 5/((s-4)**3), (3*s+4)/(s**2 + 9)]

for f in func:
    Fs = L(f);
    dp_str = f"${sympy.latex(f)} : {sympy.latex(Fs)}$"
    display(Latex(dp_str));

