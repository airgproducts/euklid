#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

#include "plane/plane.hpp"

namespace py = pybind11;
using namespace py::literals;


namespace euklid::spline {

    template<typename SplineClass>
    py::class_<SplineClass> PySpline(py::module m, const char *name) {
        return py::class_<SplineClass>(m, name)
                .def(py::init([](py::list t)
                    {
                        auto lst = get_vector_list<py::list, Vector2D>(t);
                        return SplineClass(PolyLine2D(lst));
                    }), py::arg("controlpoints"))
                .def(py::init<PolyLine2D>())
                .def("__copy__",  [](SplineClass& self) {
                    return self.copy();
                })
                .def("__deepcopy__", [](SplineClass& self, py::dict) {
                    return self.copy();
                }, "memo"_a)
                .def("__json__", [](SplineClass& self) {
                    py::dict result;
                    result["controlpoints"] = to_list<Vector2D>(self.controlpoints.nodes);
                    return result;
                })
                .def_static("fit", &SplineClass::fit, py::arg("curve"), py::arg("numpoints"))
                .def_readwrite("controlpoints", &SplineClass::controlpoints)
                .def_property("numpoints", &SplineClass::get_numpoints, &SplineClass::set_numpoints)
                .def("copy", &SplineClass::copy)
                .def("get", &SplineClass::get)
                .def("get_sequence", &SplineClass::get_sequence, py::arg("num"));
    }

    void REGISTER(pybind11::module module) {
        pybind11::module m = module.def_submodule("spline");



        PySpline<BezierCurve>(m, "BezierCurve");
        PySpline<BSplineCurve>(m, "BSplineCurve");
        PySpline<SymmetricBSplineCurve>(m, "SymmetricBSplineCurve");
        PySpline<SymmetricBezierCurve>(m, "SymmetricBezierCurve");

    }

}