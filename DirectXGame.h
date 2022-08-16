#pragma once
#include "GameBase.h"

#include "Player.h"
#include "Enemy.h"
#include "Skydome.h"
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

	//Player
	std::unique_ptr<GeometryObject3D> playerObject;
	std::unique_ptr<Player> player;

	//Enemy
	std::unique_ptr<GeometryObject3D> enemyObject;
	std::unique_ptr<Enemy> enemy;

	//Sky
	std::unique_ptr<Model> skydomeModel;		//マテリアル
	std::unique_ptr<Object3D> skydomeObject;	//オブジェクト
	std::unique_ptr<Skydome> skydome;

	//衝突判定
	std::unique_ptr<CollisionManager> collisionManager;
};

