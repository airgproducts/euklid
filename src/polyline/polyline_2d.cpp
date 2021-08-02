#include "polyline/polyline.cpp"
#include "polyline/polyline_2d.hpp"
//#include "common.cpp"


PolyLine2D PolyLine2D::normvectors() const {
    auto segments = this->get_segments();
    std::vector<std::shared_ptr<Vector2D>> segment_normals;
    std::vector<std::shared_ptr<Vector2D>> normvectors;

    for (auto segment: segments) {
        Vector2D normal;
        
        normal.set_item(0, segment->get_item(1));
        normal.set_item(1, -segment->get_item(0));

        auto normal_0 = std::make_shared<Vector2D>(normal.normalized());

        segment_normals.push_back(normal_0);
    }

    normvectors.push_back(segment_normals[0]);

    
    for (size_t i=0; i<segment_normals.size()-1; i++) {
        Vector2D normal = *segment_normals[i] + *segment_normals[i+1];

        if (normal.length() > small_d) {
            normvectors.push_back(std::make_shared<Vector2D>(normal.normalized()));
        } else {
            // n1 and n2 are opposite -> add normalized segment
            size_t index = std::max<size_t>(0, i-1);
            normvectors.push_back(std::make_shared<Vector2D>(segments[index]->normalized()));
        }
        //auto normal = segment_normals[i]->copy();

    }
    

    normvectors.push_back(segment_normals.back());

    return PolyLine2D(normvectors);
}

PolyLine2D PolyLine2D::offset(double amount) const {
    auto normvectors = this->normvectors().nodes;
    std::vector<std::shared_ptr<Vector2D>> nodes;

    for (size_t i=0; i<this->nodes.size(); i++) {
        nodes.push_back(std::make_shared<Vector2D>(*this->nodes[i] + (*normvectors[i])*amount));
    }

    return PolyLine2D(nodes);
}

static const double tolerance = 1e-5;

std::vector<std::pair<double, double>> PolyLine2D::cut(const Vector2D& p1, const Vector2D& p2) const {
    std::vector<std::pair<double, double>> results;
    if (this->nodes.size() < 2) {
        return results;
    }

    // cut first segment
    auto result = cut_2d(*this->nodes[0], *this->nodes[1], p1, p2);
    auto last_result = result;

    
    if (result.success && result.ik_1 <= 0) {
        results.push_back(std::pair<double, double>(result.ik_1, result.ik_2));
    }

    // try all segments
    for (size_t i=0; i<this->nodes.size()-1; i++) {
        result = cut_2d(*this->nodes[i], *this->nodes[i+1], p1, p2);

        if (result.success && 0. < result.ik_1 && result.ik_1 <= 1.) {
            results.push_back(std::pair<double, double>(result.ik_1+i, result.ik_2));
        } else if (-tolerance < result.ik_1 && result.ik_1 <= 0 && 1 < last_result.ik_1 && last_result.ik_1 < 1+tolerance) {
            // catch cuts falling straight on a point
            results.push_back(std::pair<double, double>(last_result.ik_1+i-1, last_result.ik_2));
        }


        last_result = result;
    }

    // add value if for the last cut ik_1 is greater than 1 (extrapolate end)
    if (result.success && result.ik_1 > 1) {
            results.push_back(std::pair<double, double>(result.ik_1+this->nodes.size()-2, result.ik_2));
    }

    return results;

}

std::pair<double, double> PolyLine2D::cut(const Vector2D& v1, const Vector2D& v2, double nearest_ik) const {
    using cut = std::pair<double, double>;
    auto cuts = this->cut(v1, v2);
    
    std::sort(cuts.begin(), cuts.end(), [nearest_ik](cut& a, cut& b) {
        return std::abs(a.first-nearest_ik) < std::abs(b.first-nearest_ik);
    });

    if (cuts.size() == 0) {
        throw std::runtime_error("No cut found!");
    }

    return cuts[0];
}

std::vector<std::pair<double, double>> PolyLine2D::cut(const PolyLine2D& l2) const {
    std::vector<std::pair<double, double>> result;


    for (size_t i=0; i<l2.nodes.size()-1; i++) {
        auto cuts = this->cut(*l2.nodes[i], *l2.nodes[i+1]);

        for (auto cut: cuts) {
            if (-tolerance < cut.second && cut.second < 1+tolerance && -tolerance < cut.first && cut.first < this->nodes.size()-1+tolerance) {
                result.push_back({cut.first, i+cut.second});
            }

        }
    }

    return result;
}

std::pair<double, double> PolyLine2D::cut(const PolyLine2D& other, double nearest_ik) const { 
    using cut = std::pair<double, double>;
    auto cuts = this->cut(other);
    
    std::sort(cuts.begin(), cuts.end(), [nearest_ik](cut& a, cut& b) {
        return std::abs(a.first-nearest_ik) < std::abs(b.first-nearest_ik);
    });

    if (cuts.size() == 0) {
        throw std::runtime_error("No cut found!");
    }

    return cuts[0];
}


PolyLine2D PolyLine2D::fix_errors() const {
    if (this->nodes.size() <= 4) {
        return this->copy();
    }
    for (size_t i=0; i<this->nodes.size()-3; i++) {
        size_t new_list_start = i+2;
        auto nodes2 = std::vector<std::shared_ptr<Vector2D>>(this->nodes.begin() + new_list_start, this->nodes.end());
        PolyLine2D line2 = PolyLine2D(nodes2);

        auto cuts = line2.cut(*this->nodes[i], *this->nodes[i+1]);
        // start from the back
        std::reverse(cuts.begin(), cuts.end());

        for (auto result: cuts) {
            if (0 <= result.first && result.first < line2.nodes.size()-1-small_d && 0 <= result.second && result.second < 1) {
                
                std::vector<std::shared_ptr<Vector2D>> new_nodes;
                // new line: 0 to i and result to end
                for (size_t j=0; j<=i; j++) {
                    new_nodes.push_back(std::make_shared<Vector2D>(*this->nodes[j]));
                }
                
                new_nodes.push_back(line2.get(result.first));

                int start_2 = int(result.first) + 1;

                if (std::abs(result.first-start_2) < tolerance) {
                    start_2 += 1;
                }

                
                for (size_t j=start_2; j<line2.nodes.size(); j++) {
                    new_nodes.push_back(std::make_shared<Vector2D>(*line2.nodes[j]));
                }

                return PolyLine2D(new_nodes).fix_errors();
            }
        }
        
    }

    // no cuts found -> remove zero-length segments

    std::vector<std::shared_ptr<Vector2D>> nodes_new;
    // Remove len-0 segment points
    auto segment_lengthes = this->get_segment_lengthes();
    nodes_new.push_back(std::make_shared<Vector2D>(*this->nodes[0]));

    for (size_t i=0; i<segment_lengthes.size(); i++){
        if (segment_lengthes[i] > tolerance) {
            nodes_new.push_back(std::make_shared<Vector2D>(*this->nodes[i+1]));
        }
    }

    return PolyLine2D(nodes_new);
}


double PolyLine2D::get_area() const {
    double area = 0;
    unsigned int j;

    for (unsigned int i=0; i<this->nodes.size(); i++) {
        j = i+1;
        if (j >= this->nodes.size()) {
            j = 0;
        }

        area += this->nodes[i]->get_item(0) * this->nodes[j]->get_item(1);
        area -= this->nodes[i]->get_item(1) * this->nodes[j]->get_item(0);
    }

    return area;
}


PolyLine2D PolyLine2D::mirror(Vector2D& p1, Vector2D& p2) const {
    auto diff = p1 - p2;
    auto normvector = Vector2D(-diff[1], diff[0]);
    std::vector<std::shared_ptr<Vector2D>> result;

    for (auto node: this->nodes) {
        result.push_back(std::make_shared<Vector2D>(
            *node - normvector * (2 * normvector.dot(*node-p1))
        ));
    }

    return PolyLine2D(result);
}


PolyLine2D PolyLine2D::rotate(double radians, Vector2D& origin) const {
    std::vector<std::shared_ptr<Vector2D>> new_nodes;
    auto rotation = Rotation2D(radians);

    for (auto node: this->nodes) {
        new_nodes.push_back(std::make_shared<Vector2D>(origin + rotation.apply(*node - origin)));
    }

    return PolyLine2D(new_nodes);
}


PolyLine3D PolyLine2D::to_3d() const {
    std::vector<std::shared_ptr<Vector3D>> new_nodes;

    return PolyLine3D();

    
}


template class PolyLine<Vector2D, PolyLine2D>;
//std::pair<std::shared_ptr<Vector2D>>