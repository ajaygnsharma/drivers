import numpy as np
import matplotlib.pyplot as plt
from matplotlib import style

# So basically take np, plt, and style.

t = np.arange(0,11); # A Range: probably array range. Not "Arrange"
print(t);

x = 0.85 ** t;
print(x); 

plt.plot(t,x, linewidth =3, label = 'x(t) = (0.85)^t')
plt.show();

x_quantized = np.around(x, 1); # Round an array to 1 decimal pt
print(x_quantized);

plt.plot(t,x_quantized, linewidth=1, label='Quantized signal');
plt.show();
