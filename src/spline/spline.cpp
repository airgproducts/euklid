#include "spline/spline.hpp"


namespace euklid::spline {

template<typename Base, typename T>
SplineCurve<Base, T>::SplineCurve(PolyLine2D controlpoints) : 
    controlpoints(controlpoints),
    base(controlpoints.nodes.size())  {}


template<typename Base, typename T>
T SplineCurve<Base, T>::copy() const {
    return T(this->controlpoints);
}

template<typename Base, typename T>
Base& SplineCurve<Base, T>::get_base() {
    if (this->base.dimension() != this->controlpoints.nodes.size()){
        this->base = Base(this->controlpoints.nodes.size());
    }

    return this->base;
}

template<typename Base, typename T>
Vector2D SplineCurve<Base, T>::get(double ik) {
    auto base = this->get_base();
    Vector2D result;
    
    for (size_t i=0; i<this->controlpoints.nodes.size(); i++) {
        result = result + *this->controlpoints.nodes[i] * base.get(i, ik);
    }

    return result;
}

template<typename Base, typename T>
PolyLine2D SplineCurve<Base, T>::get_sequence(size_t segments) {
    std::vector<std::shared_ptr<Vector2D>> nodes_new;

    for (size_t i=0; i<=segments; i++) {
        nodes_new.push_back(std::make_shared<Vector2D>(this->get(static_cast<double>(i)/segments)));
    }

    return PolyLine2D(nodes_new);
}


template<typename Base, typename T>
T SplineCurve<Base, T>::fit(const PolyLine2D& line, size_t numpoints) {
    using NodeList = std::vector<std::shared_ptr<Vector2D>>;


    if (numpoints > line.nodes.size()) {
        throw std::runtime_error("numpoints > line_points");
    }

    auto base = Base(numpoints);
    size_t rows = line.nodes.size();
    size_t columns = base.dimension();
    NodeList nodes_new;

    nodes_new.push_back(std::make_shared<Vector2D>(*line.nodes[0]));

    // Influence Matrices
    Eigen::MatrixXd A1(rows, columns-2); // nodes influence vector
    Eigen::MatrixXd A2(rows, 2);  // start/end point

    for (size_t row=0; row<rows; row++) {
        // p_0 [A1...] p_columns
        for (size_t column=1; column<columns-1; column++) {
            auto coeff = base.get(column, (double)row/(rows-1));
            A1(row, column-1) = coeff;
        }

        A2(row, 0) = base.get(0, (double)row/(rows-1));
        A2(row, 1) = base.get(columns-1, (double)row/(rows-1));
    }

    // result
    Eigen::MatrixXd p1(2, rows);
    Eigen::MatrixXd p2(2, 2);

    for (size_t dim=0; dim<2; dim++) {
        for (size_t column=0; column<rows; column++) {
            p1(dim, column) = line.nodes[column]->get_item(dim);
        }

        p2(dim, 0) = line.nodes[0]->get_item(dim);
        p2(dim, 1) = line.nodes.back()->get_item(dim);
    }

    auto A1_transposed = A1.transpose();
    auto A1_tdot = A1_transposed*A1;
    auto A1_inverse = A1_tdot.inverse();
    
    Eigen::MatrixXd solution(2, columns-2);
    for (int dim=0; dim<2; dim++) {
        auto p1_dim = p1.row(dim).transpose();
        auto rhs_1 = A1_transposed * p1_dim;
        auto p2_dim = p2.row(dim).transpose();
        auto rhs_2 = ((A1_transposed * A2) * p2_dim);

        solution.row(dim) = A1_inverse * (rhs_1-rhs_2);
    }

    for (size_t i=0; i<columns-2; i++) {
        auto vector = std::make_shared<Vector2D>();
        for (size_t dim=0; dim<2; dim++) {
            vector->set_item(dim, solution(dim, i));
        }
        nodes_new.push_back(vector);
    }

    nodes_new.push_back(std::make_shared<Vector2D>(*line.nodes.back()));

    return T(PolyLine2D(nodes_new));
}


template<typename SplineClass, typename T>
SymmetricSpline<SplineClass, T>::SymmetricSpline(PolyLine2D controlpoints) : controlpoints(controlpoints), spline_curve(controlpoints) {}


template<typename SplineClass, typename T>
T SymmetricSpline<SplineClass, T>::copy() const {
    return T(this->controlpoints);
}

template<typename SplineClass, typename T>
Vector2D SymmetricSpline<SplineClass, T>::get(double x) {
    this->apply();
    //return this->spline_curve.get(x);
    return this->spline_curve.get(0.5+x/2);
}

template<size_t degree>
Interpolation SymmetricBSplineCurve<degree>::get_curvature(size_t n) {
    this->apply();
    //return this->spline_curve.get(x);
    auto curvature = this->spline_curve.get_curvature(2*n-1);
    curvature.nodes.erase(curvature.nodes.begin(), curvature.nodes.begin()+n-1);
    
    for (auto node : curvature.nodes) {
        node->set_item(0, (node->get_item(0)-0.5)*2);
    }
    return curvature;
}


template<typename SplineClass, typename T>
PolyLine2D SymmetricSpline<SplineClass, T>::get_sequence(size_t segments) {
    this->apply();

    std::vector<std::shared_ptr<Vector2D>> nodes_new;

    for (size_t i=0; i<=segments; i++) {
        nodes_new.push_back(std::make_shared<Vector2D>(
            this->spline_curve.get(0.5+static_cast<double>(i)/(2*segments))
            //this->spline_curve.get(static_cast<double>(i)/segments)
            ));
    }

    return PolyLine2D(nodes_new);


}


template<typename SplineClass, typename T>
void SymmetricSpline<SplineClass, T>::apply() {
    Vector2D p1(0,0);
    Vector2D p2(0,1);
    auto line_1 = this->controlpoints.mirror(p1, p2);
    auto line_2 = this->controlpoints.copy();
    size_t numpoints = line_1.nodes.size();
    std::vector<std::shared_ptr<Vector2D>> nodes_new;

    for (size_t i=0; i<numpoints; i++) {
        nodes_new.insert(nodes_new.begin(), line_1.nodes[i]);
        nodes_new.push_back(line_2.nodes[i]);
    }

    this->spline_curve.controlpoints = PolyLine2D(nodes_new);    
}

template<typename SplineClass, typename T>
T SymmetricSpline<SplineClass, T>::fit(const PolyLine2D& nodes, size_t node_num) {
    Vector2D p1(0,0);
    Vector2D p2(0,1);
    auto line_1 = nodes.mirror(p1, p2);
    auto line_2 = nodes.copy();
    size_t numpoints = line_1.nodes.size();
    std::vector<std::shared_ptr<Vector2D>> nodes_new;

    nodes_new.push_back(line_1.nodes[0]);
    nodes_new.push_back(line_2.nodes[0]);
    if ((*line_2.nodes[0]-*line_1.nodes[0]).length() > 1e-6) {
    }

    for (size_t i=1; i<numpoints; i++) {
        nodes_new.insert(nodes_new.begin(), line_1.nodes[i]);
        nodes_new.push_back(line_2.nodes[i]);
    }

    auto polyline_new = PolyLine2D(nodes_new);
    
    auto spline_curve = SplineClass::fit(polyline_new, 2*node_num);

    std::vector<std::shared_ptr<Vector2D>> controlpoints_new;
    for (size_t i=0; i<node_num; i++) {
        controlpoints_new.push_back(spline_curve.controlpoints.nodes[node_num+i]);
    }

    return T(controlpoints_new);
}

template<typename SplineClass, typename T>
int SplineCurve<SplineClass, T>::get_numpoints() {
    return this->controlpoints.nodes.size();
}

template<typename SplineClass, typename T>
void SplineCurve<SplineClass, T>::set_numpoints(size_t numpoints) {
    auto nodes = this->get_sequence(100).nodes;
    auto new_spline = T::fit(nodes, numpoints);
    this->controlpoints = new_spline.controlpoints;
}

template<typename SplineClass, typename T>
int SymmetricSpline<SplineClass, T>::get_numpoints() {
    return this->controlpoints.nodes.size();
}

template<typename SplineClass, typename T>
void SymmetricSpline<SplineClass, T>::set_numpoints(size_t numpoints) {
    auto nodes = this->get_sequence(100).nodes;
    auto new_spline = T::fit(nodes, numpoints);
    this->controlpoints = new_spline.controlpoints;
}


template<size_t degree>
Interpolation BSplineCurve<degree>::get_curvature(size_t n) const {
    auto d_1 = this->get_derivate();
    auto d_2 = d_1.get_derivate();

    Interpolation result;

    for (size_t i = 0; i < n; i++) {
        float t = (float) i / (n-1);
        auto v = d_1.get(t);
        float kappa = d_2.get(t).cross(v) * (1/std::pow(v.length(), 3));
        result.nodes.push_back(std::make_shared<Vector2D>(t, kappa));

    }

    return result;

}

template<size_t degree>
std::conditional<(degree>1), BSplineCurve<degree-1>, BSplineCurve<1>>::type BSplineCurve<degree>::get_derivate() const {
    using T = std::conditional<(degree>1), BSplineCurve<degree-1>, BSplineCurve<1>>::type;
    auto controlpoints = this->controlpoints.get_segments();

    if (controlpoints.size() < 2) {
        controlpoints.push_back(std::make_shared<Vector2D>(controlpoints[0]->copy()));
    }

    return T(controlpoints);
}

/*
template<>
BSplineCurve<0> BSplineCurve<1>::get_derivate() const {
    throw std::runtime_error("Can't get derivative");
};*/


template class SplineCurve<BezierBase, BezierCurve>;
template class BSplineCurve<1>;
template class BSplineCurve<2>;
template class BSplineCurve<3>;
template class BSplineCurve<4>;
template class SplineCurve<BSplineBase<1>, BSplineCurve<1>>;
template class SplineCurve<BSplineBase<2>, BSplineCurve<2>>;
template class SplineCurve<BSplineBase<3>, BSplineCurve<3>>;
template class SplineCurve<BSplineBase<4>, BSplineCurve<4>>;
template class SymmetricBSplineCurve<2>;
template class SymmetricBSplineCurve<3>;
template class SymmetricBSplineCurve<4>;
template class SymmetricSpline<BSplineCurve<2>, SymmetricBSplineCurve<2>>;
template class SymmetricSpline<BSplineCurve<3>, SymmetricBSplineCurve<3>>;
template class SymmetricSpline<BSplineCurve<4>, SymmetricBSplineCurve<4>>;
template class SymmetricSpline<BezierCurve, SymmetricBezierCurve>;

} // namespace euklid::spline