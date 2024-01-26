"""
    Setup for ArCTIc cython wrapper. See README.md.

    Build with:
        python3 make_setup.py build_ext --inplace
"""

import os
import numpy as np
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

# Directories
dir_arctic = os.path.dirname(os.path.realpath(__file__)) + "/"
dir_arctic = "./"
dir_src = dir_arctic + "src/"
dir_include = dir_arctic + "include/"
dir_lib = dir_arctic  # Can we move this somewhere?!

dir_wrapper = dir_arctic + "python/arcticpy/"
dir_wrapper_src = dir_wrapper + "src/"
dir_wrapper_include = dir_wrapper + "include/"

# Find GSL
dir_gsl_fallback = dir_arctic + "gsl/"
if os.path.exists("/usr/local/include/gsl"):
    dir_gsl_fallback = "/usr/local/"  # brew install llvm libomp gsl
if os.path.exists("/opt/local/include/gsl"):
    dir_gsl_fallback = "/opt/local/"  # sudo port install libomp gsl
if os.path.exists("/cosma/local/gsl/2.5/lib"):
    dir_gsl_fallback = "/cosma/local/gsl/2.5/lib/"  # use on cosma
dir_gsl = os.environ.get("DIR_GSL", dir_gsl_fallback)
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
                sources=[
                    dir_wrapper + "wrapper.pyx",
                    dir_wrapper_src + "interface.cpp",
                ],
                language="c++",
                libraries=["arctic"],
                library_dirs=[dir_lib, dir_lib_gsl],
                runtime_library_dirs=[dir_lib, dir_lib_gsl],
                include_dirs=[
                    dir_include,
                    np.get_include(),
                    dir_wrapper_include,
                    dir_include_gsl,
                ],
                extra_compile_args=["-std=c++17", "-O3"],
                define_macros=[("NPY_NO_DEPRECATED_API", 0)],
            )
        ],
        compiler_directives={"language_level": "3"},
    )
)
