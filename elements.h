#ifndef SANDBOX_ELEMENTS_H
#define SANDBOX_ELEMENTS_H

#include "stdbool.h"

typedef bool (*MovementBehavior)(int ownX, int ownY);

typedef enum elemId {
    EMPTY_ELEM,
    WATER,
    SAND,
    STONE
} ElementId;

typedef enum elemType {
    EMPTY_TYPE,
    LIQUID,
    SOLID,
    GAS
} ElementType;

typedef struct element {
    ElementType type;
    bool isMovable;
    MovementBehavior move;
} Element;


#endif //SANDBOX_ELEMENTS_H
