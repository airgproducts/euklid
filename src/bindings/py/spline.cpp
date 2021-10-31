#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

#include "spline/basis.hpp"
#include "spline/spline.hpp"
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

    template<size_t degree>
    py::class_<BSplineCurve<degree>> PyBSpline(py::module m, const char *name) {
        return PySpline<BSplineCurve<degree>>(m, name)
            .def("get_derivate", &BSplineCurve<degree>::get_derivate)
            .def("get_curvature", &BSplineCurve<degree>::get_curvature, py::arg("num") = 100);

    }

    void REGISTER(pybind11::module module) {
        pybind11::module m = module.def_submodule("spline");

        PySpline<BezierCurve>(m, "BezierCurve");

        PyBSpline<1>(m, "LinSplineCurve");
        PyBSpline<2>(m, "BSplineCurve");
        PyBSpline<3>(m, "CubicBSplineCurve");
        PyBSpline<4>(m, "QuadBSplineCurve");
            
        PySpline<SymmetricBSplineCurve<2>>(m, "SymmetricBSplineCurve")
            .def("get_curvature", &SymmetricBSplineCurve<2>::get_curvature);

        PySpline<SymmetricBSplineCurve<3>>(m, "SymmetricCubicBSplineCurve")
            .def("get_curvature", &SymmetricBSplineCurve<3>::get_curvature);

        PySpline<SymmetricBSplineCurve<4>>(m, "SymmetricQuadBSplineCurve")
            .def("get_curvature", &SymmetricBSplineCurve<4>::get_curvature);

        
        PySpline<SymmetricBezierCurve>(m, "SymmetricBezierCurve");

    }

}