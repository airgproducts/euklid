#include <pybind11/pybind11.h>
#include <vector>
#include <pybind11/stl.h>

#include "version.hpp"
#include "pybind/mesh.cpp"
#include "pybind/vector.cpp"
#include "pybind/spline.cpp"
#include "pybind/plane.cpp"


namespace py = pybind11;
using namespace py::literals;


PYBIND11_MODULE(euklid, m) {
    m.doc() = "euklid vector operations";

    euklid::vector::REGISTER(m);
    euklid::mesh::REGISTER(m);
    euklid::spline::REGISTER(m);
    euklid::plane::REGISTER(m);


    m.attr("__version__") = py::str(euklid::version);
}