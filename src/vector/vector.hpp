#pragma once

#include<vector>
#include <iostream>
#include "util/math.hpp"

template<size_t dimensions, typename T>
class Vector {
    public:
        /*
        */
        Vector(const T&);
        Vector();
        Vector(double);
        
        static const int dimension = dimensions;

        double get_item(int n) const;
        void set_item(int n, double value);

        double& operator[] (int n);
        double operator[] (int n) const;
        T operator -(const T& v2) const;
        T operator +(const T& v2) const;
        T operator *(const T& v2) const;
        T operator *(const double&) const;


        double dot(const T& v2);
        //double dot(const T& v2);
        double distance(const T& v2);
        double length();

        void normalize();

        T copy();
    
    //private:
        double coordinates[dimensions];
};


class Vector2D: public Vector<2, Vector2D> {
    public:
        using Vector<2, Vector2D>::Vector;
        Vector2D();
        Vector2D(const Vector<2, Vector2D>&);
        Vector2D(double x, double y);

        double cross(const Vector2D&) const;
};

class Vector3D : public Vector<3, Vector3D> {
    public:
        using Vector<3, Vector3D>::Vector;
        Vector3D();
        Vector3D(const Vector<3, Vector3D>&);
        Vector3D(double x, double y, double z);
};


struct CutResult {
    bool success;
    double ik_1;
    double ik_2;
    Vector2D point;
};

CutResult cut_2d(const Vector2D& l1_p1, const Vector2D& l1_p2, const Vector2D& l2_p1, const Vector2D& l2_p2);
