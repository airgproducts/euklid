#include "vector/vector.hpp"

template<size_t dimensions, typename T>
Vector<dimensions, T>::Vector(const T& in) {
    for(size_t i=0; i<dimensions; i++){
        this->set_item(i, in.get_item(i));
    }
}


template<size_t dimensions, typename T>
Vector<dimensions, T>::Vector() {
    for(size_t i=0; i<dimensions; i++){
        this->set_item(i, 0);
    }
}

template<size_t dimensions, typename T>
Vector<dimensions, T>::Vector(double value) {
    for(size_t i=0; i<dimensions; i++){
        this->set_item(i, value);
    }
}


template<size_t dimensions, typename T>
double Vector<dimensions, T>::dot(const T& v2) const {
    double result = 0;

    for (size_t i=0; i<dimensions; i++) {
        result += this->get_item(i) * v2.get_item(i);
    }

    return result;
}


template<size_t dimensions, typename T>
double Vector<dimensions, T>::length() const {
    return sqrt(this->dot(static_cast<T>(*this)));
}


template<size_t dimensions, typename T>
double Vector<dimensions, T>::get_item(int n) const {
    if (n < 0 || n >= (int)dimensions) {
        throw "Invalid dimension";
    }

    return this->coordinates[n];
}


template<size_t dimensions, typename T>
void Vector<dimensions, T>::set_item(int n, double value) {
    if (n < 0 || n >= (int)dimensions) {
        throw "Invalid dimension";
    }

    this->coordinates[n] = value;
}

template<size_t dimensions, typename T>
double& Vector<dimensions, T>::operator[](int n) {
    if (n < 0 || n >= (int)dimensions) {
        throw "Invalid dimension";
    }

    return this->coordinates[n];
}


template<size_t dimensions, typename T>
double Vector<dimensions, T>::operator[](int n) const {
    return this->get_item(n);
}

template<size_t dimensions, typename T>
T Vector<dimensions, T>::operator+(const T& v2) const {
    T result;

    for (size_t i=0; i<dimensions; i++){
        result.set_item(i, this->get_item(i) + v2.get_item(i));
    }

    return result;
}

template<size_t dimensions, typename T>
T Vector<dimensions, T>::operator-(const T& v2) const {
    T result;

    for(size_t i=0; i<dimensions; i++) {
        result.set_item(i, this->get_item(i) - v2.get_item(i));
    }

    return result;
}

template<size_t dimensions, typename T>
T Vector<dimensions, T>::operator*(const T& v2) const {
    T result;

    for (size_t i=0; i<dimensions; i++){
        result.set_item(i, this->get_item(i) * v2.get_item(i));
    }

    return result;
}


template<size_t dimensions, typename T>
T Vector<dimensions, T>::operator* (const double& factor) const {
    T result;

    for (size_t i=0; i<dimensions; i++) {
        result.set_item(i, this->get_item(i)*factor);
    }

    return result;
}

template<size_t dimensions, typename T>
T Vector<dimensions, T>::operator/ (const double& factor) const {
    return *this * (1/factor);
}

template<size_t dimensions, typename T>
double Vector<dimensions, T>::distance(const T& v2) const {
    return ((*this) - v2).length();
}

template<size_t dimensions, typename T>
T Vector<dimensions, T>::copy() const {
    return T(*this);
}

template<size_t dimensions, typename T>
T Vector<dimensions, T>::normalized() const {
    T result;
    double len = this->length();

    for (size_t i=0; i<dimensions; i++) {
        result.set_item(i, this->get_item(i)/len);
    }

    return result;
}


template<size_t dimensions, typename T>
std::string Vector<dimensions, T>::repr() const {
    std::stringstream out;
    
    out << "Vector" << T::dimension << "D";

    //repr += name;
    out << "(";
    
    for (int i=0; i<T::dimension; i++) {
        out << std::fixed << std::setprecision(4) << this->get_item(i);

        if (i+1 < T::dimension) {
            out << " ";
        }
    }

    //repr.resize(repr.size()-1);
    out << ")";
    
    return out.str();
}

template<size_t dimensions, typename T>
size_t Vector<dimensions, T>::hash() const {
    size_t result = std::hash<double>{}(this->get_item(0));

    for (size_t i=1; i<dimensions; i++) {
        result = result ^ (std::hash<double>{}(this->get_item(i)) << 1);
    }

    return result;
}

Vector2D::Vector2D(double x, double y) {
    this->set_item(0, x);
    this->set_item(1, y);
}

Vector3D::Vector3D(double x, double y, double z) {
    this->set_item(0, x);
    this->set_item(1, y);
    this->set_item(2, z);
}

Vector3D::Vector3D() {
    this->set_item(0,0);
    this->set_item(1,0);
    this->set_item(2,0);
}


double Vector2D::cross(const Vector2D& v2) const {
    return (*this)[0] * v2[1] - v2[0] * (*this)[1];
}

double Vector2D::angle() const {
    return atan2(this->get_item(1), this->get_item(0));
}

Vector3D Vector3D::cross(const Vector3D& v2) const {
    Vector3D result;

    result.set_item(0,  ( (*this)[1] * v2[2] - (*this)[2] * v2[1] ));
    result.set_item(1, -( (*this)[0] * v2[2] - (*this)[2] * v2[0] ));
    result.set_item(2,  ( (*this)[0] * v2[1] - (*this)[1] * v2[0] ));

    return result;
}


Vector2D::Vector2D() {
    this->set_item(0,0);
    this->set_item(1,0);
}


Vector2D::Vector2D(const Vector<2, Vector2D>& in) {
    for(size_t i=0; i<Vector2D::dimension; i++){
        this->set_item(i, in.get_item(i));
    }
}
Vector3D::Vector3D(const Vector<3, Vector3D>& in) {
    for(size_t i=0; i<Vector3D::dimension; i++){
        this->set_item(i, in.get_item(i));
    }
}




template class Vector<3, Vector3D>;
template class Vector<2, Vector2D>;


CutResult cut_2d(const Vector2D& l1_p1, const Vector2D& l1_p2, const Vector2D& l2_p1, const Vector2D& l2_p2) {
    CutResult result;

    result.success = false;

    // Improved numerical stability constants
    static const double MIN_SEGMENT_LENGTH = 1e-8;
    static const double PARALLEL_THRESHOLD = 1e-10;

    // Compute segment vectors (KEEP THIS CONVENTION)
    Vector2D d1 = l1_p2 - l1_p1;
    Vector2D d2 = l2_p2 - l2_p1;

    double d1_len_sq = d1.dot(d1);
    double d2_len_sq = d2.dot(d2);

    // Check for degenerate segments
    if (d1_len_sq < MIN_SEGMENT_LENGTH * MIN_SEGMENT_LENGTH ||
        d2_len_sq < MIN_SEGMENT_LENGTH * MIN_SEGMENT_LENGTH) {
        return result;
    }

    // Cross product
    double cross = d1[0] * d2[1] - d1[1] * d2[0];

    // Parallel check (unchanged, fine)
    double d1_len = std::sqrt(d1_len_sq);
    double d2_len = std::sqrt(d2_len_sq);
    double cross_normalized = cross / (d1_len * d2_len);

    if (std::abs(cross_normalized) < PARALLEL_THRESHOLD) {
        return result;
    }

    // -----------------------------
    // CONSISTENT CONVENTION SECTION
    // -----------------------------

    // IMPORTANT: anchor everything to l1_p1 (same as projection method)
    Vector2D r = l2_p1 - l1_p1;

    // Solve: l1_p1 + t*d1 intersects line2
    // t = cross(r, d2) / cross(d1, d2)
    // NO extra sign flips
    result.ik_1 = (r[0] * d2[1] - r[1] * d2[0]) / cross;

    // Solve parameter on line2 in consistent orientation
    result.ik_2 = (r[0] * d1[1] - r[1] * d1[0]) / cross;

    // Compute point from line 1 (same as projection version)
    result.point = l1_p1 + d1 * result.ik_1;

    result.success = true;
    

    return result;
}