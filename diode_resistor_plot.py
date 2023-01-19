# plot for diode resistor

import matplotlib.pyplot as plt
import numpy as np

# array for resistor values
x = np.arange(0.5, 5, 0.001)

# array for source voltage values
y = np.arange(2.7, 3.6, 0.001)

# generate grid
x, y = np.meshgrid(x, y)

# values for current
# Voltage on resistor + transistor devided by resistor value + ca. 24mOhm of transistor
z = (y - 1.6) / (x + 0.024)

# create contour lines with labels
plt.clabel(plt.contour(x, y, z, np.arange(0, 3.1, 0.25), colors='black', linewidths=0.5), inline=True, fontsize=8)

plt.clabel(plt.contour(x, y, z, [0.7], colors='red', linewidths=1), inline=True, fontsize=8)

# add grid to plot
plt.grid()

# plot labels
plt.xlabel('Diode resistor [Ohm]')
plt.ylabel('Source voltage [V]')
plt.title('Current [A] in diode circuit depending\n on diode resistor and source voltage')

# show plot
plt.show()