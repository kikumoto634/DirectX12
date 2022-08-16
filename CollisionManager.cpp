#include "CollisionManager.h"
#include "Vector3.h"

void CollisionManager::SetCollision(Collider *collider)
{
	colliders.push_back(collider);
}

void CollisionManager::CollisionClear()
{
	colliders.clear();
}
