from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        name="mlx90641",
        sources=[
            "mlx90641_module.cpp",
            "functions/MLX90641_API.cpp",
            "functions/MLX90641_I2C_Driver.cpp",
        ],
        include_dirs=["headers"],
        libraries=["i2c"],       # link against libi2c (SMBus functions)
        extra_compile_args=["-std=c++11"],
    ),
]

setup(
    name="mlx90641-library",
    version="0.1.0",
    author="Your Name",
    author_email="you@example.com",
    description="Python bindings for Melexis MLX90641 thermal sensor",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    install_requires=["pybind11>=2.6.0"],
)
