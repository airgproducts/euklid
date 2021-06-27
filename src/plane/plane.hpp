#pragma once

#include "vector/transform.hpp"

namespace euklid::plane {

class Plane {
    public:
        Plane(Transformation t);
        Plane(Vector3D, Vector3D, Vector3D);

        Vector2D project(const Vector3D&) const;
        PolyLine2D project(const PolyLine3D&) const;
    
        Vector3D x_vector;
        Vector3D y_vector;
        Vector3D normvector;
        Vector3D p0;

        Transformation transformation;

    private:
        void setup();

};

}