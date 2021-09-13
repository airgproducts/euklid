#include "polyline/polyline.hpp"

template<typename VectorClass, typename T>
PolyLine<VectorClass, T>::PolyLine() : nodes() {}

template<typename VectorClass, typename T>
PolyLine<VectorClass, T>::PolyLine(std::vector<std::shared_ptr<VectorClass>>& nodes)
 : nodes(nodes) 
 {}

template<typename VectorClass, typename T>
size_t PolyLine<VectorClass, T>::hash() const {

    size_t result = this->nodes[0]->hash();

    for (size_t i=1; i<this->nodes.size(); i++) {
        result = result ^ (this->nodes[i]->hash() << 1);
    }

    return result;
}


template<typename VectorClass, typename T>
std::shared_ptr<VectorClass> PolyLine<VectorClass, T>::get(double ik) const {
    size_t i = std::max(int(ik), 0);
    VectorClass diff;

    // catch direct (int) values
    if (std::abs(ik-i) < 1e-10 && 0 <= ik && ik < this->nodes.size()) {
        return std::make_shared<VectorClass>(*this->nodes[i]);
    }

    if (i >= this->nodes.size()-1) {
        i = this->nodes.size()-1;
        diff = *this->nodes[i] - *this->nodes[i-1];
    } else {
        diff = *this->nodes[i+1] - *this->nodes[i];
    }

    double k = ik - i;
    auto p1 = this->nodes[i];

    return std::make_shared<VectorClass>(*p1 + diff*k);
}

template<typename VectorClass, typename T>
std::vector<double> PolyLine<VectorClass, T>::get_positions(double ik_start, double ik_end) const {
    std::vector<double> result;

    int direction = 1;
    bool forward = true;

    if (ik_end < ik_start) {
        direction = -1;
        forward = false;
    }

    // add first point
    result.push_back(ik_start);

    int ik = int(ik_start);

    ik = std::min(std::max((int)0, ik), (int)this->nodes.size()-2);

    if (forward) {
        ik += 1;
    }

    // todo: maybe check the length diff?
    if ((double)std::abs(ik_start-ik) < 1e-8) {
        ik += direction;
    }

    while (direction * (ik_end - ik) > 1e-8 && 0 < ik && ik < (int)this->nodes.size()-1) {
        result.push_back(ik);
        ik += direction;
    }

    result.push_back(ik_end);

    return result;
}

template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::get(double ik_start, double ik_end) const {
    auto iks = this->get_positions(ik_start, ik_end);
    std::vector<std::shared_ptr<VectorClass>> nodes_new;
    
    for (double ik: iks) {
        nodes_new.push_back(this->get(ik));
    }

    return T(nodes_new);
}

template<typename VectorClass, typename T>
std::vector<std::shared_ptr<VectorClass>> PolyLine<VectorClass, T>::get_segments() const {
    std::vector<std::shared_ptr<VectorClass>> result;
    
    if (this->nodes.size() < 2) {
        return result;
    }
    for (size_t i=0; i<this->nodes.size()-1; i++) {
        result.push_back(
            std::make_shared<VectorClass>(*this->nodes[i+1] - *this->nodes[i]));
    }
    
    
    return result;
}

template<typename VectorClass, typename T>
std::vector<std::shared_ptr<VectorClass>> PolyLine<VectorClass, T>::get_tangents() const {
    std::vector<std::shared_ptr<VectorClass>> result;
    
    if (this->nodes.size() < 2) {
        return result;
    }

    result.push_back(std::make_shared<VectorClass>((*this->nodes[1] - *this->nodes[0]).normalized()));
    
    for (size_t i=0; i<this->nodes.size()-2; i++) {
        VectorClass first = (*this->nodes[i+1] - *this->nodes[i]).normalized();
        VectorClass second = (*this->nodes[i+2] - *this->nodes[i+1]).normalized();
        VectorClass tangent = first + second;
        
        double length = tangent.length();

        if (length <= small_d) {
            tangent = first;
        }

        result.push_back(std::make_shared<VectorClass>(tangent));
    }

    result.push_back(std::make_shared<VectorClass>((*this->nodes[this->nodes.size()-1] - *this->nodes[this->nodes.size()-2]).normalized()));

    return result;

}

template<typename VectorClass, typename T>
std::vector<double> PolyLine<VectorClass, T>::get_segment_lengthes() const {
    std::vector<double> result;

    for (auto segment: this->get_segments()) {
        result.push_back(segment->length());
    }

    return result;
}

template<typename VectorClass, typename T>
double PolyLine<VectorClass, T>::get_length() {
    auto segments = this->get_segments();
    double length = 0;
    for (auto segment: segments) {
        length += segment->length();
    }

    return length;
}

template<typename VectorClass, typename T>
double PolyLine<VectorClass, T>::walk(double start, double amount) {
    if (std::abs(amount) < 1e-5) {
        return start;
    }

    int direction = 1;
    if (amount < 0) {
        direction = -1;
    }

    int next_value = int(start);
    if (direction > 0) {
        next_value += 1;
    }
    if (start < 0) {
        next_value -= 1;
    }
    
    if ((double)std::abs(start - next_value) < 1e-5){
        next_value += direction;
    }

    amount = (double)std::abs(amount);

    double current_segment_length = (*this->get(next_value) - *this->get(start)).length();
    amount -= current_segment_length;
    
    while (amount > 0) {
        if (next_value > (int)this->nodes.size() && direction > 0) {
            break;
        }
        if (next_value < 0 && direction < 0) {
            break;
        }

        start = next_value;
        next_value += direction;
        current_segment_length = (*this->get(next_value) - *this->get(start)).length();

        amount -= current_segment_length;
    }

    return next_value + direction * amount * (double)std::abs(next_value - start) / current_segment_length;
}


template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::resample(const int num_points) {
    double distance = this->get_length() / (num_points-1);
    double ik = 0;
    std::vector<std::shared_ptr<VectorClass>> nodes_new;

    nodes_new.push_back(this->get(0.));
    
    for (int i=0; i<num_points-2; i++) {
        ik = this->walk(ik, distance);
        nodes_new.push_back(this->get(ik));
    }

    nodes_new.push_back(std::make_shared<VectorClass>(*this->nodes.back()));

    return T(nodes_new);
}

template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::copy() const {
    std::vector<std::shared_ptr<VectorClass>> nodes_new;

    for (auto node: this->nodes) {
        nodes_new.push_back(std::make_shared<VectorClass>(*node));
    }

    return T(nodes_new);
}


template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::scale(const VectorClass& scale) const {
    std::vector<std::shared_ptr<VectorClass>> nodes_new;

    for (auto node: this->nodes) {
        nodes_new.push_back(std::make_shared<VectorClass>(*node * scale));
    }

    return T(nodes_new);
}


template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::scale(const double scale) const {
    auto scale_vector = VectorClass();
    for (int i=0; i<VectorClass::dimension; i++) {
        scale_vector.set_item(i, scale);
    }

    return this->scale(scale_vector);
}


template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::mix(T& other, const double amount) const {
    if (other.nodes.size() != this->nodes.size()){
        throw "PolyLine sizes don't match!";
    }

    T line_1 = this->scale(1-amount);
    T line_2 = other.scale(amount);

    std::vector<std::shared_ptr<VectorClass>> nodes_new;

    for (size_t i=0; i<this->nodes.size(); i++) {
        auto node = *line_1.nodes[i] + *line_2.nodes[i];
        nodes_new.push_back(std::make_shared<VectorClass>(node));
    }

    return T(nodes_new);
}


template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::add(const T& other) const {
    std::vector<std::shared_ptr<VectorClass>> new_nodes;

    if (this->nodes.size() != other.nodes.size()){
        throw std::runtime_error("PolyLine sizes don't match!");
    }

    for (unsigned int i=0; i<this->nodes.size(); i++) {
        auto node = *this->nodes[i] + *other.nodes[i];
        new_nodes.push_back(std::make_shared<VectorClass>(node));
    }

    return T(new_nodes);
}


template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::sub(const T& other) const {
    return this->add(other.scale(-1));
}

template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::scale_nodes(const std::vector<double>& other) const {
    std::vector<std::shared_ptr<VectorClass>> new_nodes;

    if (this->nodes.size() != other.size()){
        throw std::runtime_error("PolyLine sizes don't match!");
    }

    for (unsigned int i=0; i<this->nodes.size(); i++) {
        auto node = *this->nodes[i] * other[i];
        new_nodes.push_back(std::make_shared<VectorClass>(node));
    }

    return T(new_nodes);
}


template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::operator+ (const T& line2) const {
    std::vector<std::shared_ptr<VectorClass>> new_nodes;

    for (auto node: this->nodes) {
        new_nodes.push_back(std::make_shared<VectorClass>(*node));
    }

    for (auto node: line2.nodes) {
        new_nodes.push_back(std::make_shared<VectorClass>(*node));
    }

    return T(new_nodes);
}

template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::move (const VectorClass& offset) const {
    std::vector<std::shared_ptr<VectorClass>> new_nodes;

    for (auto node: this->nodes) {
        new_nodes.push_back(std::make_shared<VectorClass>(*node + offset));
    }

    return T(new_nodes);
}


template<typename VectorClass, typename T>
T PolyLine<VectorClass, T>::reverse() const {
    return this->get(this->nodes.size()-1, 0);
}


template<typename VectorClass, typename T>
int PolyLine<VectorClass, T>::numpoints() {
    return this->nodes.size();
}


template class PolyLine<Vector3D, PolyLine3D>;