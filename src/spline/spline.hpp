#pragma once

#include <functional>
#include <Eigen/Dense>

#include "vector/vector.hpp"
#include "polyline/polyline_2d.hpp"
#include "spline/basis.hpp"


namespace euklid::spline {
        

template<typename Base, typename T>
class SplineCurve {
    public:
        SplineCurve(PolyLine2D);

        Vector2D get(double);
        PolyLine2D get_sequence(size_t);

        PolyLine2D controlpoints;
        T copy() const;

        static T fit(const PolyLine2D&, size_t);

        void set_numpoints(size_t);
        int get_numpoints();
    
    private:
        Base& get_base();
        Base base;
};

class BezierCurve : public SplineCurve<BezierBase, BezierCurve> {
        using SplineCurve<BezierBase, BezierCurve>::SplineCurve;
};
class BSplineCurve : public SplineCurve<BSplineBase<2>, BSplineCurve> {
        using SplineCurve<BSplineBase<2>, BSplineCurve>::SplineCurve;
};

template<typename SplineClass, typename T>
class SymmetricSpline {
    public:
        SymmetricSpline(PolyLine2D);

        Vector2D get(double);
        PolyLine2D get_sequence(size_t);

        PolyLine2D controlpoints;
        T copy() const;

        static T fit(const PolyLine2D&, size_t);
        
        void set_numpoints(size_t);
        int get_numpoints();
    
    private:
        void apply();
        SplineClass spline_curve;
};

class SymmetricBSplineCurve : public SymmetricSpline<BSplineCurve, SymmetricBSplineCurve> {
        using SymmetricSpline<BSplineCurve, SymmetricBSplineCurve>::SymmetricSpline;
};

class SymmetricBezierCurve : public SymmetricSpline<BezierCurve, SymmetricBezierCurve> {
        using SymmetricSpline<BezierCurve, SymmetricBezierCurve>::SymmetricSpline;
};

} // namespace euklid::spline