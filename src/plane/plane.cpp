#include "plane/plane.hpp"

namespace euklid::plane {

    Plane::Plane(Transformation t) : transformation(t) {
        this->setup();
    }


    Plane::Plane(Vector3D p0, Vector3D v1, Vector3D v2) {
        Vector3D n = v1.cross(v2);

        for (size_t i=0; i<3; i++) {
            this->transformation.matrix[i][0] = v1.get_item(i); // x
            this->transformation.matrix[i][1] = v2.get_item(i); // y
            this->transformation.matrix[i][2] = n.get_item(i);  // z

            this->transformation.matrix[3][i] = p0.get_item(i);
        }

        this->setup();
    }

    void Plane::setup() {
        this->p0 = this->transformation.apply(Vector3D(0,0,0));
        this->x_vector = this->transformation.apply(Vector3D(1, 0, 0)) - this->p0;
        this->y_vector = this->transformation.apply(Vector3D(0, 1, 0)) - this->p0;
        this->normvector = this->transformation.apply(Vector3D(0, 0, 1)) - this->p0;
    }

    Vector2D Plane::project(const Vector3D& vec) const {
        Vector3D diff = vec - this->p0;

        double x = this->x_vector.dot(diff);
        double y = this->y_vector.dot(diff);

        return Vector2D(x, y);
    }

    PolyLine2D Plane::project(const PolyLine3D& line) const {
        PolyLine2D result;

        for (auto vec: line.nodes) {
            result.nodes.push_back(std::make_shared<Vector2D>(this->project(*vec)));
        }

        return result;
    }




    
}