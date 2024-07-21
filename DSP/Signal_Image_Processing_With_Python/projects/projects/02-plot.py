#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed May 29 22:52:13 2024

@author: asharma
"""

import numpy as np
import matplotlib.pyplot as plt

x = np.linspace(0, 5, 10);
y = x ** 2;

# Here the plot is red in color.
plt.plot(x,y, 'r');
plt.show(x,y);
