import os

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

dir_root = os.path.dirname(os.path.realpath(__file__)) + "/"
dir_arctic = os.path.abspath(os.path.join(dir_root, os.pardir)) + "/"
dir_include = dir_arctic + "include/"
dir_link = dir_arctic + "build/"

# Clean
for root, dirs, files in os.walk(dir_root, topdown=False):
    for name in files:
        file = os.path.join(root, name)
        if name.endswith(".o") or (
            name.startswith("wrapper")
            and not (name.endswith(".pyx") or name.endswith(".pxd"))
        ):
            print("rm", file)
            os.remove(file)

# Build
setup(
    ext_modules=cythonize(
        [
            Extension(
                "wrapper",
                sources=["wrapper.pyx", "interface.cpp"],
                language="c++",
                libraries=["arctic"],
                library_dirs=[dir_link],
                runtime_library_dirs=[dir_link],
                include_dirs=[dir_include],
                # extra_compile_args=["-I%s" % dir_include],
                # extra_link_args=["-L%s -Wl,-rpath=%s -larctic" % (dir_link, dir_link)],
            )
        ],
        compiler_directives={"language_level": "3"},
    )
)
