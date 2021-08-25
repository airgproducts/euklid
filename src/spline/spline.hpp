#pragma once

#include <functional>
#include <Eigen/Dense>

#include "vector/vector.hpp"
#include "polyline/interpolation.hpp"
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

template<size_t degree>
class BSplineCurve : public SplineCurve<BSplineBase<degree>, BSplineCurve<degree>> {
    using SplineCurve<BSplineBase<degree>, BSplineCurve<degree>>::SplineCurve;
    
    public:
        Interpolation get_curvature(size_t) const;
        std::conditional<(degree>1), BSplineCurve<degree-1>, BSplineCurve<1>>::type get_derivate() const;
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
    
    protected:
        void apply();
        SplineClass spline_curve;
};

template<size_t degree>
class SymmetricBSplineCurve : public SymmetricSpline<BSplineCurve<degree>, SymmetricBSplineCurve<degree>> {
    using SymmetricSpline<BSplineCurve<degree>, SymmetricBSplineCurve<degree>>::SymmetricSpline;
    public:
        Interpolation get_curvature(size_t);
};

class SymmetricBezierCurve : public SymmetricSpline<BezierCurve, SymmetricBezierCurve> {
    using SymmetricSpline<BezierCurve, SymmetricBezierCurve>::SymmetricSpline;
};

} // namespace euklid::spline