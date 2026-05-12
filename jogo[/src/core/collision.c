#include "collision.h"

Rectangle getColliderRect(Vector2 position, Collider collider) {
    return (Rectangle) {
        position.x + collider.offset.x,
        position.y + collider.offset.y,
        collider.size.x,
        collider.size.y
    };
}

int rectsCollide(Rectangle a, Rectangle b) {
    return CheckCollisionRecs(a, b);
}

int collidesWithAny(Rectangle rect, const Rectangle* blockers, int blockerCount) {
    if (!blockers || blockerCount <= 0) {
        return 0;
    }

    for (int i = 0; i < blockerCount; i++) {
        if (rectsCollide(rect, blockers[i])) {
            return 1;
        }
    }

    return 0;
}

Vector2 resolveMovement(Rectangle colliderRect, Vector2 desiredMove, const Rectangle* blockers, int blockerCount) {
    Vector2 resolvedMove = { 0 };
    Rectangle candidate = colliderRect;

    candidate.x += desiredMove.x;
    if (!collidesWithAny(candidate, blockers, blockerCount)) {
        resolvedMove.x = desiredMove.x;
    }

    candidate = colliderRect;
    candidate.x += resolvedMove.x;
    candidate.y += desiredMove.y;
    if (!collidesWithAny(candidate, blockers, blockerCount)) {
        resolvedMove.y = desiredMove.y;
    }

    return resolvedMove;
}