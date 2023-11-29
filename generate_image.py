# /usr/bin/env python3

import numpy as np
from numpy import random

import sys

dims = np.array(sys.argv[1:3], dtype=int)

print(
    f"Producing test image with {dims[0]} rows and {dims[1]} columns", file=sys.stderr
)

mean = 2.35e3
variance = 0.03e3

image = random.normal(mean, variance, dims)
image = np.abs(image) + 0.01e3

print(f"{dims[0]} {dims[1]}")
np.savetxt(sys.stdout, image)
