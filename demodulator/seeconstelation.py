#!/usr/bin/env python

import struct
import numpy as np
import matplotlib.pyplot as plt


f = open("test.bin", "rb")
data = f.read()
f.close()

length = len(data) / 8 # Number of complex

print "Symbols: %s" %length

x = []
y = []

for i in range(length): # Number of complexes
  real, imag = struct.unpack("2f", data[i*8:i*8+8])
  x.append(real)
  y.append(imag)

plt.ylim([-1,1])
plt.xlim([-1,1])
plt.grid(True)
plt.scatter(y, x)
plt.show()