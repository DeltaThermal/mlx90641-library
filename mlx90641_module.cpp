// mlx90641_module.cpp
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <chrono>
#include <thread>
#include <stdexcept>

#include "MLX90641_API.h"
#include "MLX90641_I2C_Driver.h"

namespace py = pybind11;
static const uint8_t MLX_ADDR = 0x33;

// Calibration & frame buffers
static paramsMLX90641 mlx90641;
static uint16_t eeMLX90641[832];
static uint16_t frame[834];
static float mlx90641To[192];
static constexpr float emissivity = 1.0f;

// Poll STATUS_REG (0x8000) bit 3 until data ready
inline void wait_for_data_ready() {
    uint16_t status = 0;
    do {
        // Read one word from STATUS_REG (0x8000)
        MLX90641_I2CRead(MLX_ADDR, 0x8000, 1, &status);
    } while ((status & (1 << 3)) == 0);
}

void setup(int fps) {
    // Initialize I2C and reset sensor
    MLX90641_I2CInit();
    MLX90641_I2CGeneralReset();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Configure refresh rate bits (Control Register 1 bits [9:7])
    uint8_t code;
    switch (fps) {
        case 1:   code = 0b000; break;
        case 2:   code = 0b001; break;
        case 4:   code = 0b010; break;
        case 8:   code = 0b011; break;
        case 16:  code = 0b101; break;
        case 32:  code = 0b110; break;
        case 64:  code = 0b111; break;
        default:  code = 0b101; break; // default to 16Hz
    }
    MLX90641_SetRefreshRate(MLX_ADDR, code);

    // Read & decode EEPROM calibration data
    MLX90641_DumpEE(MLX_ADDR, eeMLX90641);
    eeMLX90641[10] |= (1 << 6);  // patch known bad bit
    int err = MLX90641_ExtractParameters(eeMLX90641, &mlx90641);
    if (err != 0) {
        throw std::runtime_error("Parameter extraction failed: " + std::to_string(err));
    }

    // Wait for first valid full-frame (Tvalid = 80ms + 2*(1000ms/fps))
    std::this_thread::sleep_for(
        std::chrono::milliseconds(80) +
        std::chrono::milliseconds(static_cast<int>(2000.0f / fps))
    );

    // Sync to first frame
    MLX90641_SynchFrame(MLX_ADDR);
}

void cleanup() {}

py::array_t<float> get_frame() {
    // Wait then read subpage 0
    wait_for_data_ready();
    int stat = MLX90641_GetFrameData(MLX_ADDR, frame);
    if (stat != 0) {
        throw std::runtime_error("GetFrameData subpage 0 failed: " + std::to_string(stat));
    }

    // Wait then read subpage 1
    wait_for_data_ready();
    stat = MLX90641_GetFrameData(MLX_ADDR, frame);
    if (stat != 0) {
        throw std::runtime_error("GetFrameData subpage 1 failed: " + std::to_string(stat));
    }

    // Calculate ambient & object temperatures
    float Ta = MLX90641_GetTa(frame, &mlx90641);
    MLX90641_CalculateTo(frame, &mlx90641, emissivity, Ta, mlx90641To);
    MLX90641_BadPixelsCorrection(mlx90641.brokenPixel, mlx90641To);

    // Return as 12x16 NumPy array (rows, cols)
    return py::array_t<float>(
        {12, 16},                            // shape: rows, cols
        {16 * sizeof(float), sizeof(float)}, // strides
        mlx90641To                           // data pointer
    );
}

PYBIND11_MODULE(mlx90641, m) {
    m.doc() = "pybind11 bindings for Melexis MLX90641 thermal sensor";
    m.def("setup", &setup, "Initialize sensor at given FPS", py::arg("fps")=16);
    m.def("get_frame", &get_frame, "Grab one 12x16 temperature frame (°C)");
    m.def("cleanup", &cleanup, "Cleanup resources (noop)");
}
