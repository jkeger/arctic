"""
    Setup for ArCTIc cython wrapper. See README.md.

    Build with:
        python3 setup.py build_ext --inplace
"""

import os
import numpy as np
from setuptools import setup, Extension
from Cython.Build import cythonize

# Directories
dir_wrapper = "python/arcticpy/"
dir_wrapper_src = "python/arcticpy/src/"
dir_wrapper_include = "python/arcticpy/include/"
dir_include = "include/"
dir_src = "src/"
# dir_gsl = dir_arctic + "gsl/"
# dir_include_gsl = dir_gsl + "include/"
# dir_lib_gsl = dir_gsl + "lib/"

# Clean (really needed anymore?)
for root, dirs, files in os.walk(dir_wrapper, topdown=False):
    for name in files:
        file = os.path.join(root, name)
        if name.endswith(".o") or (
            name.startswith("wrapper")
            and not (name.endswith(".pyx") or name.endswith(".pxd"))
        ):
            print("rm", file)
            os.remove(file)

# Build
if "CC" not in os.environ:
    os.environ["CC"] = "g++"

ext_headers = [os.path.join(dir_include, header) for header in os.listdir(dir_include)]
ext_sources = [os.path.join(dir_src, src) for src in os.listdir(dir_src)]

extensions = [
    Extension(
        name="arcticpy.libarctic",
        sources=[dir_wrapper_src + "wrapper.pyx", dir_wrapper_src + "interface.cpp", *ext_sources],
        language="c++",
        libraries=["gsl"],
        runtime_library_dirs=[],
        include_dirs=[dir_wrapper_include, dir_include, np.get_include()],
        extra_compile_args=["-std=c++17", "-O3"],
        define_macros=[('NPY_NO_DEPRECATED_API', 0)],
    ),
]

setup(
    ext_modules=cythonize(
        extensions,
        compiler_directives={"language_level": "3"}
    ),
    headers=ext_headers,  # currently ignored (?)
)
