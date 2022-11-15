#include "polyline/polyline.cpp"
#include "polyline/polyline_2d.hpp"
//#include "common.cpp"


PolyLine2D PolyLine2D::segment_normals() const {
    auto segments = this->get_segments();
    std::vector<std::shared_ptr<Vector2D>> segment_normals;

    for (auto segment: segments) {
        Vector2D normal;
        
        normal.set_item(0, segment->get_item(1));
        normal.set_item(1, -segment->get_item(0));

        auto normal_0 = std::make_shared<Vector2D>(normal.normalized());

        segment_normals.push_back(normal_0);
    }

    return PolyLine2D(segment_normals);

}


PolyLine2D PolyLine2D::normvectors() const {
    auto segments = this->get_segments();
    auto segment_normals = this->segment_normals().nodes;

    std::vector<std::shared_ptr<Vector2D>> normvectors;



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

PolyLine2D PolyLine2D::offset(double amount, bool simple=true) const {
    if (simple) {
        auto normvectors = this->normvectors().nodes;
        std::vector<std::shared_ptr<Vector2D>> nodes;

        for (size_t i=0; i<this->nodes.size(); i++) {
            nodes.push_back(std::make_shared<Vector2D>(*this->nodes[i] + (*normvectors[i])*amount));
        }

        return PolyLine2D(nodes);
    } else {
        using Segment = std::pair<std::shared_ptr<Vector2D>, std::shared_ptr<Vector2D>>;
        auto segments = this->get_segments();
        std::vector<std::shared_ptr<Vector2D>> segments_normalized;
        PolyLine2D result;

        for (auto segment: segments) {
            segments_normalized.push_back(std::make_shared<Vector2D>(segment->normalized()));
        }


        auto segment_normals = this->segment_normals().nodes;
        std::vector<Segment> offset_segments;

        for (size_t i=0; i<this->nodes.size()-1; i++) {

            offset_segments.push_back(Segment {
                std::make_shared<Vector2D>(*this->nodes[i] + (*segment_normals[i])*amount),
                std::make_shared<Vector2D>(*this->nodes[i+1] + (*segment_normals[i])*amount)
            });
        }

        result.nodes.push_back(std::make_shared<Vector2D>(
            *this->nodes[0] + *segment_normals[0] * amount
        ));

        for (size_t i=0; i<this->nodes.size()-2; i++) {
            auto segment_1 = segments_normalized[i];
            auto segment_2 = segments_normalized[i+1];
            double cos_angle = segment_1->dot(*segment_2);
            if (cos_angle > 0.99) {
                result.nodes.push_back(std::make_shared<Vector2D>(
                    (*offset_segments[i].second + *offset_segments[i+1].first)/2)
                );
            } else {
                
                auto cut = cut_2d(
                    *offset_segments[i].first, *offset_segments[i].second,
                    *offset_segments[i+1].first, *offset_segments[i+1].second
                );
                if (cut.success && cut.ik_1 > 0. && cut.ik_2 < 1.) {
                    result.nodes.push_back(std::make_shared<Vector2D>(cut.point));
                } else if (result.nodes.size() > 2) {
                    
                    try {
                        auto cut = result.cut(*offset_segments[i+1].first, *offset_segments[i+1].second, result.nodes.size()-1);

                        if (cut.second < 1) {
                            if (cut.first < result.nodes.size()-1) {
                                result = result.get(0, cut.first);
                            } else {
                                result.nodes.push_back(result.get(cut.first));
                            }
                        }
                    }
                    catch (std::runtime_error&) {

                    }
                }
#
                // todo: make a circle
            }
        }

        result.nodes.push_back(std::make_shared<Vector2D>(
            *this->nodes.back() + *segment_normals.back() * amount
        ));

        return PolyLine2D(result);
    }
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

    
    if (result.success && result.ik_1 <= tolerance) {
        results.push_back(std::pair<double, double>(result.ik_1, result.ik_2));
    }

    // try all segments
    for (size_t i=0; i<this->nodes.size()-1; i++) {
        result = cut_2d(*this->nodes[i], *this->nodes[i+1], p1, p2);

        if (result.success && tolerance < result.ik_1 && result.ik_1 <= 1.-tolerance) {
            results.push_back(std::pair<double, double>(result.ik_1+i, result.ik_2));
        } else if (-tolerance < result.ik_1 && result.ik_1 <= tolerance && 1.-tolerance < last_result.ik_1 && last_result.ik_1 <= 1+tolerance) {
            // catch cuts falling straight on a point
            results.push_back(std::pair<double, double>(static_cast<double>(i), last_result.ik_2));
        }


        last_result = result;
    }

    // add value if for the last cut ik_1 is greater than 1 (extrapolate end)
    if (result.success && result.ik_1 > 1.-tolerance) {
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

    return std::abs(area/2);
}

std::vector<std::shared_ptr<Vector2D>> PolyLine2D::boundary() const {
    double min_x = std::numeric_limits<double>::infinity();
    double max_x = -std::numeric_limits<double>::infinity();
    double min_y = min_x;
    double max_y = max_x;

    for (auto vec: this->nodes) {
        min_x = std::min<double>(min_x, vec->get_item(0));
        max_x = std::max<double>(max_x, vec->get_item(0));
        min_y = std::min<double>(min_y, vec->get_item(1));
        max_y = std::max<double>(max_y, vec->get_item(1));
    }

    std::vector<std::shared_ptr<Vector2D>> boundary;

    boundary.push_back(std::make_shared<Vector2D>(min_x, min_y));
    boundary.push_back(std::make_shared<Vector2D>(max_x, min_y));
    boundary.push_back(std::make_shared<Vector2D>(max_x, max_y));
    boundary.push_back(std::make_shared<Vector2D>(min_x, max_y));

    return boundary;
}

bool PolyLine2D::contains(const Vector2D& p1) const {
    // todo check boundary before

    for (auto vec: this->boundary()) {
        auto diff = Vector2D(1,0);
        //auto diff = (*vec-p1);
        if (diff.length() > 1e-3) {
            unsigned int valid_cuts = 0;
            auto p2 = p1 + diff*2;

            auto cuts = this->cut(p1, p2);

            if (cuts.size() == 0) {
                return false;
            }

            for (auto cut: cuts) {
                if (cut.first < 0) {}
                else if (cut.first >= this->nodes.size()-1) {}
                else if (cut.second < 0) {}
                else {
                    valid_cuts += 1;
                }
            }
            
            return valid_cuts % 2 > 0;

        }

    }

    return false;
}

PolyLine2D PolyLine2D::close() const {
    PolyLine2D new_line = this->copy();

    if ((*new_line.nodes.back() - *new_line.nodes.front()).length() > small_d) {
        new_line.nodes.push_back(new_line.nodes.front());
    }


    return new_line;
}

std::vector<PolyLine2D> PolyLine2D::bool_union(const PolyLine2D& other) const {
    using cut = std::pair<double, double>;
    std::vector<PolyLine2D> result;

    auto first = this->close();
    auto second = other.close();

    auto cuts = first.cut(second);

    if (cuts.size() == 0) {
        if (first.contains(*second.nodes[0])) {
            result.push_back(second);
        } else if (second.contains(*first.nodes[0])) {
            result.push_back(first);
        }
    } else {
        auto new_line = new PolyLine2D();
        
        std::sort(cuts.begin(), cuts.end(), [](cut& a, cut& b) {
            return a.first < b.first;
        });


        cut start = {0, 0};
        cut end = {first.nodes.size()-1, second.nodes.size()-1};

        
        if (cuts.size() > 1) {
            if (cuts[0].second > cuts[1].second) {
                start.second = second.nodes.size() - 1;
            }
            if (cuts.back().second < cuts[cuts.size()-2].second) {
                end.second = 0;
            }
        }
        cuts.insert(cuts.begin(), start);
        cuts.push_back(end);

        //cut start_positions = {0, 0};

        for (unsigned int i = 0; i < cuts.size()-1; i++) {
            auto start_positions = cuts[i];
            auto end_positions = cuts[i+1];

            float ik_middle = (std::get<0>(start_positions) + std::get<0>(end_positions))/2;
            bool first_inside_second = second.contains(*first.get(ik_middle));
            bool second_inside_first = false;
            unsigned int node_offset = new_line->nodes.size() > 0 ? 1 : 0;

            ik_middle = (std::get<1>(start_positions) + std::get<1>(end_positions))/2;
            second_inside_first = first.contains(*second.get(ik_middle));

            if (i > 0 && first_inside_second && second_inside_first) {
                auto to_insert = first.get(std::get<0>(start_positions), std::get<0>(end_positions));
                new_line->nodes.insert(new_line->nodes.end(), to_insert.nodes.begin(), to_insert.nodes.end());
                to_insert = second.get(std::get<1>(end_positions), std::get<1>(start_positions));
                new_line->nodes.insert(new_line->nodes.end(), to_insert.nodes.begin()+1, to_insert.nodes.end());

                result.push_back(new_line->close());
                new_line = new PolyLine2D();
            }

            else if (first_inside_second) {
                auto to_insert = first.get(std::get<0>(start_positions), std::get<0>(end_positions));
                new_line->nodes.insert(new_line->nodes.end(), to_insert.nodes.begin() + node_offset, to_insert.nodes.end());
            } else if (second_inside_first) {
                auto to_insert = second.get(std::get<1>(start_positions), std::get<1>(end_positions));
                new_line->nodes.insert(new_line->nodes.end(), to_insert.nodes.begin() + node_offset, to_insert.nodes.end());
            }

            //start_positions = cuts[i];
        }

        if (new_line->nodes.size() > 0) {
            result.push_back(*new_line);
        }
    }

    


    return result;

}

PolyLine2D PolyLine2D::mirror(Vector2D& p1, Vector2D& p2) const {
    auto diff = p1 - p2;
    auto normvector = Vector2D(-diff[1], diff[0]).normalized();
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
