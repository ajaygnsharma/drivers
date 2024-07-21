#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jun 21 18:31:37 2024

@author: asharma
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib import style

# So basically take np, plt, and style.
x = np.arange(-10,10); # A Range: probably array range. Not "Arrange"

y = (x-1)*(x-2)/x;


plt.plot(x,y)
plt.show();

