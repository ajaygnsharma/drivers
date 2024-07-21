# -*- coding: utf-8 -*-

from sympy import *

q = symbols('q');

# ex 1
expr = (3*q + 4/q + 3 - 5*q - (1/q) - 1 );
display(expr);

# ex 2
expr = 2*q + (3*q**2) - (5/q) - (4/(q**3));
display(expr);

#-- Seems like nothing happens. The expression is already simplified.
simp = simplify(expr);
display(simp);

canc = cancel(expr);
display(canc);


# ex 3
expr = (sqrt(3) + sqrt(15)*q)/(sqrt(2) + sqrt(10)*q)
display(expr);
simp = simplify(expr);
display(simp);

#substitute value of q as 2.
y = expr.subs(q,2);
display(y);
display(y.evalf());
