//
// Created by jammer on 03/04/17.
//

#include "Collision.h"
#include <assert.h>

using namespace sf;

const float epsilon = 0.001f;
const int max_it = 20;

ColResult static_collide(const ColShape &a, const ColShape &b) {
    ColResult result;
    WVec v = a.get_support(WVec(1, 0)) - b.get_support(WVec(-1, 0));
    Simplex s;
    WVec w = a.get_support(-v) - b.get_support(v);
    int it = 0;
    while ((dot(v, v) - dot(v, w)) > epsilon && it < max_it) {
        s.add(w);
        /*	if (dot(v, w) > 0)
                break;*/
        s.solve(WVec(0, 0));
        v = s.get_closest();
        if (dot(v, v) == 0.f)
            break;
        w = a.get_support(-v) - b.get_support(v);
        ++it;
    }
    result.collides = dot(v, v) == 0.f;
    if (result.collides) {
        WVec normal;
        result.e = find_closest_edge(s).distance;
        result.depth = find_normal_epa(a, b, s, normal);
        result.normal = normal;
    }
    else
        result.depth = sqrtf(dot(v, v));
    return result;
}

float find_normal_epa(const ColShape &a, const ColShape &b, Simplex &s, WVec &normal) {
    float dist = std::numeric_limits<float>::max();
    int it = 0;
    while (true) {
        auto e = find_closest_edge(s);
        auto p = a.get_support(e.normal) - b.get_support(-e.normal);
        float d = dot(p, e.normal);
        float test = d - e.distance;
        if (test < epsilon || test == dist || it > 5) {
            normal = e.normal;
            return d;
        }
        s.add(p, e.index);
        dist = test < dist ? test : dist;
        ++it;
    }
}

Edge find_closest_edge(const Simplex &s) {
    Edge edge;
    edge.distance = std::numeric_limits<float>::max();

    for (unsigned int i = 0; i < s.verts.size(); ++i) {
        int j = i + 1 == s.verts.size() ? 0 : i + 1;
        auto a = s.verts[i];
        auto b = s.verts[j];

        auto e = b - a;
        auto n = triple_prod(e, a, e);
        //normalize
        //n /= sqrt(dot(n, n));
        n /= sqrtf(pow(n.x, 2.f) + powf(n.y, 2.f));

        auto d = dot(n, a);
        if (d < edge.distance) {
            edge.distance = d;
            edge.normal = n;
            edge.index = j;
        }
    }

    return edge;
}

void Simplex::solve(const WVec & x) {
    switch (count) {
        case 1:
            return;

        case 2: {
            solve2(x);
            return;
        }

        case 3: {
            solve3(x);
            return;
        }

        default:
            assert(false);
    }
}

bool Simplex::add(const WVec &v, int index) {
    switch (count) {
        case 0: {
            verts[0] = v;
            ++count;
            break;
        }

        case 1: {
            if (v == verts[0])
                return false;
            verts[1] = v;
            ++count;
            break;
        }

        case 2: {
            if (v == verts[0] || v == verts[1])
                return false;
            verts[2] = v;
            ++count;
            break;
        }

        default: {
            //assert(false);
            verts.insert(verts.begin() + index, v);
            ++count;
        }
    }
    return true;
}

WVec Simplex::get_closest() const {
    switch (count) {
        case 1:
            return verts[0];

        case 2: {
            float s = 1.0f / denom;
            return (s * barys[0]) * verts[0] + (s * barys[1]) * verts[1];
        }

        case 3:
            return WVec(0, 0);

        default:
            assert(false);
            return WVec(0, 0);
    }

}

void Simplex::solve2(const WVec & x) {

    // Compute barycentric coordinates (pre-division).
    float u = dot(x - verts[1], verts[0] - verts[1]);
    float v = dot(x - verts[0], verts[1] - verts[0]);

    // Region A
    if (v <= 0.0f) {
        // Simplex is reduced to just vertex A.
        barys[0] = 1.0f;
        denom = 1.0f;
        count = 1;
        return;
    }

    // Region B
    if (u <= 0.0f) {
        // Simplex is reduced to just vertex B.
        // We move vertex B into vertex A and reduce the count.
        verts[0] = verts[1];
        barys[0] = 1.0f;
        denom = 1.0f;
        count = 1;
        return;
    }

    // Region AB. Due to the conditions above, we are
    // guaranteed the the edge has non-zero length and division
    // is safe.
    barys[0] = u;
    barys[1] = v;
    auto e = verts[1] - verts[0];
    denom = dot(e, e);
    count = 2;
}

void Simplex::solve3(const WVec & x) {
    // Compute edge barycentric coordinates (pre-division).
    float uAB = dot(x - verts[1], verts[0] - verts[1]);
    float vAB = dot(x - verts[0], verts[1] - verts[0]);

    float uBC = dot(x - verts[2], verts[1] - verts[2]);
    float vBC = dot(x - verts[1], verts[2] - verts[1]);

    float uCA = dot(x - verts[0], verts[2] - verts[0]);
    float vCA = dot(x - verts[2], verts[0] - verts[2]);

    // Region A
    if (vAB <= 0.0f && uCA <= 0.0f) {
        barys[0] = 1.0f;
        denom = 1.0f;
        count = 1;
        return;
    }

    // Region B
    if (uAB <= 0.0f && vBC <= 0.0f) {
        verts[0] = verts[1];
        barys[0] = 1.0f;
        denom = 1.0f;
        count = 1;
        return;
    }

    // Region C
    if (uBC <= 0.0f && vCA <= 0.0f) {
        verts[0] = verts[2];
        barys[0] = 1.0f;
        denom = 1.0f;
        count = 1;
        return;
    }

    // Compute signed triangle area.
    float area = cross(verts[1] - verts[0], verts[2] - verts[0]);

    // Compute triangle barycentric coordinates (pre-division).
    float uABC = cross(verts[1] - x, verts[2] - x);
    float vABC = cross(verts[2] - x, verts[0] - x);
    float wABC = cross(verts[0] - x, verts[1] - x);

    // Region AB
    if (uAB > 0.0f && vAB > 0.0f && wABC * area <= 0.0f) {
        barys[0] = uAB;
        barys[1] = vAB;
        auto e = verts[1] - verts[0];
        denom = dot(e, e);
        count = 2;
        return;
    }

    // Region BC
    if (uBC > 0.0f && vBC > 0.0f && uABC * area <= 0.0f) {
        verts[0] = verts[1];
        verts[1] = verts[2];

        barys[0] = uBC;
        barys[1] = vBC;
        //auto e = verts[2] - verts[1];
        auto e = verts[1] - verts[0];
        denom = dot(e, e);
        count = 2;
        return;
    }

    // Region CA
    if (uCA > 0.0f && vCA > 0.0f && vABC * area <= 0.0f) {
        verts[1] = verts[0];
        verts[0] = verts[2];

        barys[0] = uCA;
        barys[1] = vCA;
        //auto e = verts[0] - verts[2];
        auto e = verts[1] - verts[0];
        denom = dot(e, e);
        count = 2;
        return;
    }

    // Region ABC
    // The triangle area is guaranteed to be non-zero.
    //assert(uABC > 0.0f && vABC > 0.0f && wABC > 0.0f);
    barys[0] = uABC;
    barys[1] = vABC;
    barys[2] = wABC;
    denom = area;
    count = 3;
}

WVec find_directed_overlap(const ColResult &result, const WVec &direction) {
    auto dir = w_normalize(direction);
    auto projection = dot(-dir, result.normal);
    if (abs(projection) < epsilon) {
        return result.normal * result.depth;
    }
    return dir * (result.depth / projection - .01f); // - result.normal * .001f;
}