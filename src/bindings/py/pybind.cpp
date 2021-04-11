#include <pybind11/pybind11.h>
#include <vector>
#include <pybind11/stl.h>

#include "version.hpp"
#include "bindings/py/mesh.cpp"
#include "bindings/py/vector.cpp"
#include "bindings/py/spline.cpp"
#include "bindings/py/plane.cpp"


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