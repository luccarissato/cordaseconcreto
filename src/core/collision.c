#include "collision.h"
#include "raymath.h"

Rectangle getColliderRect(Vector2 position, Collider collider) {
    return (Rectangle) {
        position.x + collider.offset.x,
        position.y + collider.offset.y,
        collider.size.x,
        collider.size.y
    };
}

float getRectEdgeDistance(Rectangle rect1, Rectangle rect2) {
    float gapX = 0.0f;
    if (rect1.x + rect1.width < rect2.x) {
        gapX = rect2.x - (rect1.x + rect1.width);
    } else if (rect2.x + rect2.width < rect1.x) {
        gapX = rect1.x - (rect2.x + rect2.width);
    }
    
    float gapY = 0.0f;
    if (rect1.y + rect1.height < rect2.y) {
        gapY = rect2.y - (rect1.y + rect1.height);
    } else if (rect2.y + rect2.height < rect1.y) {
        gapY = rect1.y - (rect2.y + rect2.height);
    }
    
    if (gapX <= 0.0f && gapY <= 0.0f) {
        return 0.0f;
    }
    
    if (gapX < 0.0f) gapX = 0.0f;
    if (gapY < 0.0f) gapY = 0.0f;
    
    return sqrtf(gapX * gapX + gapY * gapY);
}

int areCollidersNearEdgeBased(Vector2 pos1, Collider collider1, Vector2 pos2, Collider collider2, float maxDistance) {
    Rectangle rect1 = getColliderRect(pos1, collider1);
    Rectangle rect2 = getColliderRect(pos2, collider2);

    float distance = getRectEdgeDistance(rect1, rect2);

    return distance <= maxDistance;
}

int rectsCollide(Rectangle a, Rectangle b) {
    return CheckCollisionRecs(a, b);
}

// rectsCollideTouching - Verifica colisão INCLUSIVE toque nas bordas
static int rectsCollideTouching(Rectangle a, Rectangle b) {
    return (a.x < b.x + b.width &&
            a.x + a.width >= b.x &&
            a.y < b.y + b.height &&
            a.y + a.height >= b.y);
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