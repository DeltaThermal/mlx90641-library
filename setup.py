from setuptools import setup, Extension
import os
import sys
from setuptools.command.build_ext import build_ext

class get_pybind_include(object):
    """Helper class to determine the pybind11 include path"""
    def __str__(self):
        import pybind11
        return pybind11.get_include()

ext_modules = [
    Extension(
        'mlx90641_cpp',
        sources=[
    'mlx90641/bindings.cpp',
    'functions/MLX90641_API.cpp',
],
        include_dirs=[
            'headers',
            get_pybind_include()
        ],
        language='c++',
        extra_compile_args=['-std=c++11'],
    ),
]

setup(
    name='mlx90641',
    version='0.1.0',
    author='Wesley Newman',
    description='Python bindings for MLX90641 driver (C++ backend)',
    long_description=open("README.md").read() if os.path.exists("README.md") else "",
    ext_modules=ext_modules,
    packages=['mlx90641'],
    install_requires=['pybind11'],
    zip_safe=False,
    python_requires='>=3.6',
)
