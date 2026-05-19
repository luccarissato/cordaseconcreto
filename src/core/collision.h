#ifndef COLLISION_H
#define COLLISION_H

#include "raylib.h"

typedef struct {
    Vector2 offset;
    Vector2 size;
} Collider;

Rectangle getColliderRect(Vector2 position, Collider collider);

float getRectEdgeDistance(Rectangle rect1, Rectangle rect2);

int areCollidersNearEdgeBased(Vector2 pos1, Collider collider1, Vector2 pos2, Collider collider2, float maxDistance);

int rectsCollide(Rectangle a, Rectangle b);
int collidesWithAny(Rectangle rect, const Rectangle* blockers, int blockerCount);
Vector2 resolveMovement(Rectangle colliderRect, Vector2 desiredMove, const Rectangle* blockers, int blockerCount);

#endif