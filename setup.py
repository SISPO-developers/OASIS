from setuptools import Extension, setup
from Cython.Build import cythonize
import numpy

ext_modules = [
    Extension("python_to_c",
              sources=["python_to_c.pyx"],
              include_dirs=[".",numpy.get_include()],
              extra_compile_args=["-O3"])
]

setup(name="python_to_c",
      ext_modules=cythonize(ext_modules))