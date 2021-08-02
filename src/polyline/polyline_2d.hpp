#pragma once

#include "util/math.hpp"
#include "vector/vector.hpp"
#include "polyline/polyline.hpp"
#include "vector/rotation_2d.hpp"

class PolyLine2D : public PolyLine<Vector2D, PolyLine2D> {
    public:
        using PolyLine<Vector2D, PolyLine2D>::PolyLine;
        
        //PolyLine2D resample(int num_points);
        PolyLine2D normvectors() const;
        PolyLine2D offset(double amount) const;

        std::vector<std::pair<double, double>> cut(const Vector2D& p1, const Vector2D& p2) const;
        std::pair<double, double> cut(const Vector2D& p1, const Vector2D& p2, double nearest_ik) const;
        std::vector<std::pair<double, double>> cut(const PolyLine2D& l2) const;
        std::pair<double, double> cut(const PolyLine2D& l2, double nearest_ik) const;
        PolyLine2D fix_errors() const;

        double get_area() const;

        PolyLine2D mirror(Vector2D& p1, Vector2D& p2) const;
        PolyLine2D rotate(double, Vector2D&) const;

        PolyLine3D to_3d() const;
};