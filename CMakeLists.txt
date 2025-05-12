cmake_minimum_required(VERSION 3.4)
project(mlx90641_py)

find_package(pybind11 REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/headers)

pybind11_add_module(mlx90641
    mlx90641_module.cpp
    functions/MLX90641_API.cpp
    functions/MLX90641_I2C_Driver.cpp
)

# if your API needs -lm or other libs:
# target_link_libraries(mlx90641 PRIVATE m)
