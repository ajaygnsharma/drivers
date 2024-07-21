#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun 22 00:02:20 2024

@author: asharma
"""
import sympy
sympy.init_printing();

import tbcontrol
tbcontrol.expectversion('0.1.2');

s, z, a = sympy.symbols('s, z, a');
k = sympy.Symbol('k', integer=True);
Dt = sympy.Symbol('\Delta t', positive=True);

unit_step=sympy.Sum(k**2/z**k, (k, 0, sympy.oo));
display(unit_step)

short_form=unit_step.doit();
display(short_form);

