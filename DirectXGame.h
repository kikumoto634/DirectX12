﻿#pragma once
#include "GameBase.h"

#include "Player.h"
#include "Enemy.h"
#include "Collider.h"

class DirectXGame : public GameBase
{

public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

	/// <summary>
	/// 衝突判定と応答
	/// </summary>
	void CheckAllCollision();

private:
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

private:

	std::unique_ptr<GeometryObject3D> playerObject;
	std::unique_ptr<Player> player;

	std::unique_ptr<GeometryObject3D> enemyObject;
	std::unique_ptr<Enemy> enemy;
};

