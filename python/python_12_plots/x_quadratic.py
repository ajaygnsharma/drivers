import matplotlib.pyplot as plt
import numpy as np

# create 1000 equally spaced points between -10 and 10
x = np.linspace(-10, 10, 100)

# calculate the y value for each element of the x vector
#y = x**2 + 2*x + 2
#y = x**2
#y = x**3 + (4 * (x**2)) + (2*x) + 2
#y = (x**4) + (3 * (x**3)) + (2 * (x**2)) + (x) + 2
y = (x**5) + (4 * (x**4)) + (3 * (x**3)) + (2 * (x**2)) + (x) + 2

fig, ax = plt.subplots()
ax.plot(x, y);
plt.show()
