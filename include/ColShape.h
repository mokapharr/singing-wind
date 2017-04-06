//
// Created by jammer on 03/04/17.
//

#ifndef SINGING_WIND_COLSHAPE_H
#define SINGING_WIND_COLSHAPE_H

#include "WindDefs.h"
#include <array>

const float line_triangulate_split = 150.f;

struct ColResult;

class ColShape {
public:
    virtual WVec get_support(const WVec &dir) const = 0;
    virtual float get_radius() const = 0;
    virtual ColResult collides(const ColShape &other) const;
    virtual void add_gfx_lines(sf::VertexArray &lines_va, const WTransform &transform) = 0;
    virtual void transform(const WTransform &transform) = 0;

    // in local space
    WVec m_center;
    bool m_highlight = false;
};

class ColTriangle : public ColShape {
public:
    WVec get_support(const WVec &dir) const override;
    float get_radius() const override {return m_radius;}
    virtual void add_gfx_lines(sf::VertexArray &lines_va, const WTransform &tf) override;
    virtual void transform(const WTransform &transform) override;
    ColTriangle(const WVec &p1, const WVec &p2, const WVec &p3);
    ~ColTriangle() = default;

private:
    std::array<WVec, 3> m_vertices;
    float m_radius;
};

class ColCircle : public ColShape {
public:
    WVec get_support(const WVec &dir) const override;
    float get_radius() const override {return m_radius;}
    virtual void add_gfx_lines(sf::VertexArray &lines_va, const WTransform &tf) override;
    virtual void transform(const WTransform &transform) override;

    ColCircle(float radius);
    ~ColCircle() = default;

private:
    float m_radius;
};

class ColCapsule : public ColShape {
public:
    WVec get_support(const WVec &dir) const override;
    float get_radius() const override {return m_radius;}
    virtual void add_gfx_lines(sf::VertexArray &lines_va, const WTransform &tf) override;
    virtual void transform(const WTransform &transform) override;

    ColCapsule(float radius, float length);
    ~ColCapsule() = default;

private:
    float m_capsule_radius;
    float m_radius;
    WVec m_a;
    WVec m_b;
};


#endif //SINGING_WIND_COLSHAPE_H