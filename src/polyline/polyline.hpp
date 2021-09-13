#pragma once

#include "util/math.hpp"
#include<vector>
#include <memory>

#include "vector/vector.hpp"


template<typename VectorType, typename T>
    class PolyLine {
        public:
            PolyLine();
            PolyLine(std::vector<std::shared_ptr<VectorType>>&);

            std::shared_ptr<VectorType> get(double ik) const;
            T get(double ik_start, double ik_end) const;
            std::vector<double> get_positions(double ik_start, double ik_end) const;

            int numpoints();
            double get_length();
            std::vector<std::shared_ptr<VectorType>> get_segments() const;
            std::vector<double> get_segment_lengthes() const;
            std::vector<std::shared_ptr<VectorType>> get_tangents() const;
            
            double walk(double start, double amount);

            std::vector<std::shared_ptr<VectorType>> nodes;

            T resample(const int num_points);
            T reverse() const;
            T copy() const;
            T scale(const VectorType&) const;
            T scale(const double) const;
            T mix(T&, const double) const;
            T move(const VectorType&) const;
            T add(const T&) const;
            T sub(const T&) const;
            T scale_nodes(const std::vector<double>&) const;

            T operator+ (const T&) const;
            T operator- (const T&) const;

            size_t hash() const;            
    };

class PolyLine3D : public PolyLine<Vector3D, PolyLine3D> {
    public:
        using PolyLine<Vector3D, PolyLine3D>::PolyLine;
};
