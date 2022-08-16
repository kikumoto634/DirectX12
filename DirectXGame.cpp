#include "DirectXGame.h"


using namespace std;

void DirectXGame::Initialize()
{
	//基底クラスの初期化
	GameBase::Initialize();

#pragma region シーンの初期化

	//スプライト共通テクスチャ読込
	textureManager->LoadTexture(1, L"Resources/Texture.jpg");
	textureManager->LoadTexture(2, L"Resources/Texture2.jpg");


	playerObject = make_unique<GeometryObject3D>();
	player = make_unique<Player>();
	player->Initialize(input.get(), 1, playerObject.get());

	enemyObject = make_unique<GeometryObject3D>();
	enemy = make_unique<Enemy>();
	enemy->Initialize(2, enemyObject.get(), {0, 25, 500});
	enemy->SetPlayer(player.get());

	skydomeModel = make_unique<Model>();
	skydomeModel = unique_ptr<Model>(FbxLoader::GetInstance()->LoadModeFromFile("skydome"));
	skydomeObject = make_unique<Object3D>();
	skydome = make_unique<Skydome>();
	skydome->Initialize(skydomeModel.get(), skydomeObject.get());


	collisionManager = make_unique<CollisionManager>();

#pragma endregion

}

void DirectXGame::Update()
{
	//基底クラスの更新
	GameBase::Update();

#pragma region シーン更新

	//カメラ更新
	camera->Update();

	//更新
	player->Update();
	if(enemy)
	{
		enemy->Update();
	}

	skydome->Update();

	CheckAllCollision();
#pragma endregion
}

void DirectXGame::Draw()
{
	//DirectXCommon前処理
	dxCommon->BeginDraw();


	//3D描画
	player->Draw(dxCommon->GetCommandList());
	if(enemy)
	{
		enemy->Draw(dxCommon->GetCommandList());
	}

	skydome->Draw(dxCommon->GetCommandList());
	
	//DirectXCommon描画後処理
	dxCommon->EndDraw();
}

void DirectXGame::Finalize()
{
	//基底クラスの解放
	GameBase::Finalize();

}

void DirectXGame::CheckAllCollision()
{
	//リストクリア
	collisionManager->CollisionClear();

	//自弾リストの取得
	const std::list<std::unique_ptr<PlayerBullet>>& playerBullets = player->GetBullets();
	//敵弾リストの取得
	const std::list<std::unique_ptr<EnemyBullet>>& enemyBullets = enemy->GetBullets();


	//コライダーをリストに登録
	collisionManager->SetCollision(player.get());
	collisionManager->SetCollision(enemy.get());
	//自弾すべてについて
	for(const std::unique_ptr<PlayerBullet>& bullet : playerBullets){
		collisionManager->SetCollision(bullet.get());
	}
	//敵弾すべてについて
	for(const std::unique_ptr<EnemyBullet>& bullet : enemyBullets){
		collisionManager->SetCollision(bullet.get());
	}

	//総当たり判定
	//リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = collisionManager->colliders.begin();
	for(; itrA != collisionManager->colliders.end(); ++itrA){
		//イテレータAからコライダーAを取得
		Collider* colliderA = *itrA;

		//イテレータBはイテレータAの次の要素から回す(重複判定回避)
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for(; itrB != collisionManager->colliders.end(); ++itrB){
			//イテレータBからコライダーBを取得
			Collider* colliderB = *itrB;

			//ペアの当たり判定
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void DirectXGame::CheckCollisionPair(Collider *colliderA, Collider *colliderB)
{
	//衝突フィルタリング
	if(colliderA->GetCollisionAttribute() == colliderB->GetCollisionMask() || colliderB->GetCollisionAttribute() == colliderA->GetCollisionMask()){
		return;
	}

	Vector3 posA = colliderA->GetPosition();
	Vector3 posB = colliderB->GetPosition();

	float length = (posB.x-posA.x)*(posB.x-posA.x) + (posB.y-posA.y)*(posB.y-posA.y) + (posB.z-posA.z)*(posB.z-posA.z);
	if(length <= (3+3)*(3+3)){
		colliderA->OnCollision();
		colliderB->OnCollision();
	}
}

