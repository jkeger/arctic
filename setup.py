"""
    Setup for ArCTIc cython wrapper. See README.md.

    Build with:
        python3 setup.py build_ext --inplace
"""

import os
import numpy as np
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

# Directories
dir_arctic = os.path.dirname(os.path.realpath(__file__)) + "/"
dir_wrapper = os.path.dirname(os.path.realpath(__file__)) + "/python/arcticpy/"
dir_include = dir_arctic + "include/"
dir_src = dir_arctic + "src/"
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

ext_libraries = [["arctic", {
    'include_dirs': [dir_include],
    'libraries': ["gsl"],
    'library_dirs': [dir_lib],
    'sources': [os.path.join(dir_src, src) for src in os.listdir(dir_src)],
}]]

extensions = [
    Extension(
        name="arcticpy.wrapper",
        sources=[dir_wrapper + "wrapper.pyx", dir_wrapper + "interface.cpp"],
        language="c++",
        libraries=["arctic", "gsl"],
        runtime_library_dirs=[dir_lib, dir_lib_gsl],
        include_dirs=[dir_include, np.get_include(), dir_include_gsl],
        extra_compile_args=["-std=c++11", "-O3"],
        define_macros=[('NPY_NO_DEPRECATED_API', 0)],
    ),
]

setup(
    name='arcticpy',
    packages=['arcticpy'],
    package_dir={'': 'python'},
    ext_modules=cythonize(extensions, compiler_directives={"language_level": "3"}),
    libraries=ext_libraries
)
