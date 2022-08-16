#pragma once
#include "GameBase.h"

#include "Player.h"
#include "Enemy.h"
#include "Collider.h"
#include "CollisionManager.h"

class DirectXGame : public GameBase
{

public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	/// <summary>
	/// 衝突判定と応答
	/// </summary>
	void CheckAllCollision();

	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);


private:

	std::unique_ptr<GeometryObject3D> playerObject;
	std::unique_ptr<Player> player;

	std::unique_ptr<GeometryObject3D> enemyObject;
	std::unique_ptr<Enemy> enemy;

	//衝突判定
	std::unique_ptr<CollisionManager> collisionManager;
};

