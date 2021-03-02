#include "polyline/polyline_2d.hpp"


class Interpolation : public PolyLine2D {
    public:
        //using PolyLine<Vector2D, PolyLine2D>::PolyLine;
        using PolyLine2D::PolyLine2D;
        Interpolation(std::vector<std::shared_ptr<Vector2D>>&, bool);

        double get_value(double) const;

        bool extrapolate = true;

        Interpolation operator*(double) const;
        Interpolation operator+(const Interpolation&) const;

        Interpolation copy() const;
};