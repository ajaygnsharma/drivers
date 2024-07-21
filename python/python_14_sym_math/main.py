from sympy import *

init_printing();

print(sqrt(3));


print(2 * sqrt(8));



x, y = symbols('x y');
expr = x + 2 * y;
print(expr);

print(expr + 1);

print(expr -x);
print(x*(expr))

expr_ = expand(x * expr);
print(expr_);

print(factor(expr_))




x, t, z, nu = symbols('x t z nu');
init_printing(use_unicode=True);



print(diff(sin(x)*exp(x), x));

print(solve(x**2 -2, x))

print(sympify('1/2 + 2/4', evaluate=False))
print(sympify('1/2 + 2/4', evaluate=True))

print(sympify('3/2 + 2/3', evaluate=False))
print(sympify('3/2 + 2/3', evaluate=True))

print(lcm(3,2));

print(solve(x*5/2 + 1/2));


# Using Substitution
expr = x*2 + 1;
print(expr)

print(expr.subs(x, 2));
print(expr.subs(x, -2));
print(expr.subs(x, pi));

expr = (2*x + 5 - 15);


print(solve(2*x + 5 - 15, x));

# Solve Equation:
print(solve(Eq(2*x + 5, 25)));





