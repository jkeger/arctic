"""
    Setup for ArCTIC cython wrapper. See README.md.

    Build with:
        python3 setup.py build_ext --inplace
"""

import os
import numpy as np
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

# Directories
dir_wrapper = os.path.dirname(os.path.realpath(__file__)) + "/"
dir_src = dir_wrapper + "src/"
dir_arctic = os.path.abspath(os.path.join(dir_wrapper, os.pardir)) + "/"
dir_include = dir_arctic + "include/"
dir_lib = dir_arctic
dir_gsl = dir_arctic + "gsl/"
dir_include_gsl = dir_gsl + "include/"
dir_lib_gsl = dir_gsl + "lib/"

# Clean
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
setup(
    ext_modules=cythonize(
        [
            Extension(
                "wrapper",
                sources=[dir_src + "wrapper.pyx", dir_src + "interface.cpp"],
                language="c++",
                libraries=["arctic"],
                library_dirs=[dir_lib, dir_lib_gsl],
                runtime_library_dirs=[dir_lib, dir_lib_gsl],
                include_dirs=[dir_include, np.get_include(), dir_include_gsl],
                extra_compile_args=["-std=c++11", "-O3"],
                define_macros=[('NPY_NO_DEPRECATED_API', 0)],
            )
        ],
        compiler_directives={"language_level": "3"},
    )
)
