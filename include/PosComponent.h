#ifndef POSCOMPONENT_H
#define POSCOMPONENT_H

#include "WindDefs.h"

struct PosComponent {
    WVec position = {0, 0};
    float rotation = 0.f;
    WTransform global_transform = WTransform();
    WTransform global_inverse_transform = WTransform();
    unsigned int parent = 0;
};

class GameWorld;
void build_global_transform(GameWorld &, unsigned int);
#endif /* POSCOMPONENT_H */
