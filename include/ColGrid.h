//
// Created by jammer on 04/04/17.
//

#ifndef SINGING_WIND_COLGRID_H
#define SINGING_WIND_COLGRID_H

#include "WindDefs.h"
#include "ColShape.h"
#include <unordered_map>
#include <set>
#include <memory>
#include <vector>

const int P1 = 73856093;
const int P2 = 19349663;

struct StaticTriangle {
    WVec center;
    float radius;
    std::shared_ptr<ColTriangle> shape;
    unsigned int id;
};

struct DynamicEntity {
    WVec center;
    float radius;
    unsigned int entity;
};

template <typename grid_object>
class HashGrid {
public:
    HashGrid(unsigned int size=static_cast<unsigned int>(c_line_triangulate_split * 1.5f)) :
        m_cell_size(size) {
    }
    ~HashGrid() = default;

    void add_object(grid_object object) {
        size_t pos = m_objects.size();
        auto ids = get_ids_for_object(object.center, object.radius);
        m_objects.push_back(std::move(object));
        for (auto id : ids)
            m_buckets[id].push_back(pos);
    }

    void clear() {
        m_buckets = std::unordered_map<int, std::vector<size_t>>();
        m_objects.clear();
    }

    std::vector<grid_object> find_colliders_in_radius(const WVec &center, float radius) {
        std::vector<grid_object> colliders;
        std::set<size_t> indices;
        for (const auto &id : get_ids_for_object(center, radius)) {
            for (size_t i : m_buckets[id]) {
                indices.insert(i);
            }
        }
        for (auto i : indices) {
            colliders.push_back(m_objects[i]);
        }
        return colliders;
    }

    std::vector<grid_object> get_objects() const {
        return m_objects;
    }

    std::vector<grid_object> get_colliders_of_point(const WVec &p) {
        std::vector<grid_object> colliders;
        auto id = get_id(p);
        for (auto i : m_buckets[id]) {
            colliders.push_back(m_objects[i]);
        }
        return colliders;
    }

    unsigned int get_size() const {return m_cell_size;}

    std::vector<grid_object> insert_and_find_colliders(grid_object object) {
        auto ids = get_ids_for_object(object.center, object.radius);
        std::vector<grid_object> colliders;

        std::set<size_t> indices;
        for (const auto &id : ids) {
            for (size_t i : m_buckets[id]) {
                indices.insert(i);
            }
        }
        for (auto i : indices) {
            colliders.push_back(m_objects[i]);
        }

        // actually add
        size_t pos = m_objects.size();
        m_objects.push_back(std::move(object));
        for (auto id : ids)
            m_buckets[id].push_back(pos);

        return colliders;
    }

private:
    unsigned int m_cell_size;
    std::vector<grid_object> m_objects;
    std::unordered_map<int, std::vector<size_t>> m_buckets;

    int get_id(const WVec &v) const {
        int x = static_cast<int>(v.x / m_cell_size);
        int y = static_cast<int>(v.y / m_cell_size);
        return x * P1 ^ y * P2;
    }

    std::set<int> get_ids_for_object(const WVec &center, float radius) const {
        std::set<int> ids;
        int times = static_cast<int>(2.f * radius / m_cell_size) + 1;
        for (int i = 0; i <= times; ++i) {
            for (int j = 0; j <= times; ++j) {
                int id = get_id(center + WVec(static_cast<float>(i * m_cell_size) - radius,
                                              static_cast<float>(j * m_cell_size) - radius));
                ids.insert(id);
            }
        }
        return ids;
    }
};


#endif //SINGING_WIND_COLGRID_H
