#include "vector/transform.hpp"

namespace euklid::plane {

class Plane {
    public:
        Plane(Transformation t);
        Plane(Vector3D, Vector3D, Vector3D);

        Vector3D align(Vector3D&) const;
        Vector3D align(Vector2D&) const;

        Vector2D project(Vector3D&) const;

        Vector3D p0() const;
        Vector3D x_vector() const;
        Vector3D y_vector() const;
        Vector3D normvector() const;

        Transformation transformation;

    private:

};

}