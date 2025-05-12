#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "MLX90641_API.h"

namespace py = pybind11;

// Wrap struct so Python can manage it
PYBIND11_MODULE(mlx90641_cpp, m) {
    py::class_<paramsMLX90641>(m, "ParamsMLX90641")
        .def(py::init<>());

    m.def("dump_ee", [](uint8_t addr) {
        auto arr = py::array_t<uint16_t>(832);
        int err = MLX90641_DumpEE(addr, arr.mutable_data());
        return py::make_tuple(err, arr);
    }, "Dump EEPROM data");

    m.def("get_frame_data", [](uint8_t addr) {
        auto arr = py::array_t<uint16_t>(242);  // 192 + 48 + 2
        int err = MLX90641_GetFrameData(addr, arr.mutable_data());
        return py::make_tuple(err, arr);
    }, "Get frame data from MLX90641");

    m.def("extract_parameters", [](py::array_t<uint16_t> eeData, paramsMLX90641 &params) {
        return MLX90641_ExtractParameters(eeData.mutable_data(), &params);
    }, "Extract calibration parameters");

    m.def("calculate_to", [](py::array_t<uint16_t> frameData,
                             const paramsMLX90641 &params,
                             float emissivity, float tr) {
        auto to = py::array_t<float>(192);
        MLX90641_CalculateTo(frameData.mutable_data(), &params, emissivity, tr, to.mutable_data());
        return to;
    }, "Calculate temperature output");

    m.def("get_image", [](py::array_t<uint16_t> frameData,
                          const paramsMLX90641 &params) {
        auto img = py::array_t<float>(192);
        MLX90641_GetImage(frameData.mutable_data(), &params, img.mutable_data());
        return img;
    }, "Calculate raw image output");

    m.def("get_subpage", &MLX90641_GetSubPageNumber, "Get subpage number");
    m.def("get_emissivity", &MLX90641_GetEmissivity, "Get emissivity");
    m.def("get_vdd", &MLX90641_GetVdd, "Get Vdd");
    m.def("get_ta", &MLX90641_GetTa, "Get ambient temperature");
}
