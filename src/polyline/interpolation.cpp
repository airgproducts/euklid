#include "polyline/interpolation.hpp"

Interpolation::Interpolation(std::vector<std::shared_ptr<Vector2D>>& nodes, bool extrapolate) : 
    PolyLine2D(nodes),
    extrapolate(extrapolate)
    {}


double Interpolation::get_value(double x) const {
    Vector2D p1(x, 0);
    Vector2D p2(x, 1);
    auto cuts = this->cut(p1, p2);
    size_t size = this->nodes.size();

    if (cuts.size() <= 0) {
        throw std::runtime_error("No match!");
    }

    for (auto& cut: cuts) {
        bool contained = cut.first >= 0 && cut.first <= size - 1;
        if (this->extrapolate || contained) {
            return this->get(cut.first)->get_item(1);
        }
    }
    
    throw std::runtime_error("No match!");

}


Interpolation Interpolation::operator*(double scale) const {
    std::vector<std::shared_ptr<Vector2D>> nodes_new;

    for (auto node: this->nodes) {
        nodes_new.push_back(std::make_shared<Vector2D>(node->get_item(0), node->get_item(1)*scale));
    }

    return Interpolation(nodes_new);
}

Interpolation Interpolation::operator+(const Interpolation& other) const {
    std::vector<std::shared_ptr<Vector2D>> nodes_new;

    for (auto node: this->nodes) {
        double x = node->get_item(0);
        nodes_new.push_back(std::make_shared<Vector2D>(
            x,
            node->get_item(1) + other.get_value(x)
            ));
    }

    return Interpolation(nodes_new);
}

Interpolation Interpolation::copy() const {
    return *(this) * 1.;

}