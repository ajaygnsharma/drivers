import numpy as np
import matplotlib.pyplot as plt

x = np.linspace(0, 5, 10);
y = 2 * x;


plt.plot(x, y);
plt.plot(y,x);
plt.show();
