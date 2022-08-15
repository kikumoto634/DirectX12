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
	//判定対象AとBの座標
	Vector3 posA, posB;

	//自弾リストの取得
	const std::list<std::unique_ptr<PlayerBullet>>& playerBullets = player->GetBullets();
	//敵弾リストの取得
	const std::list<std::unique_ptr<EnemyBullet>>& enemyBullets = enemy->GetBullets();

#pragma region 自キャラと敵弾の当たり判定

	posA = player.get()->GetPosition();
	for(const std::unique_ptr<EnemyBullet>& bullet : enemyBullets){
		posB = bullet.get()->GetPosition();

		float length = (posB.x-posA.x)*(posB.x-posA.x) + (posB.y-posA.y)*(posB.y-posA.y) + (posB.z-posA.z)*(posB.z-posA.z);
		if(length <= (2+2)*(2+2))
		{
			player->OnCollision();
			bullet->OnCollision();
		}
	}

#pragma endregion

#pragma region 自弾と敵キャラの当たり判定

	posA = enemy.get()->GetPosition();
	for(const std::unique_ptr<PlayerBullet>& bullet : playerBullets){
		posB = bullet.get()->GetPosition();

		float length = (posB.x-posA.x)*(posB.x-posA.x) + (posB.y-posA.y)*(posB.y-posA.y) + (posB.z-posA.z)*(posB.z-posA.z);
		if(length <= (5+5)*(5+5))
		{
			enemy->OnCollision();
			bullet->OnCollision();
		}
	}

#pragma endregion

#pragma region 自弾と敵弾の当たり判定

	for(const std::unique_ptr<PlayerBullet>& playerBullet : playerBullets){
		for(const std::unique_ptr<EnemyBullet>& enemyBullet : enemyBullets){

			posA = playerBullet.get()->GetPosition();
			posB = enemyBullet.get()->GetPosition();

			float length = (posB.x-posA.x)*(posB.x-posA.x) + (posB.y-posA.y)*(posB.y-posA.y) + (posB.z-posA.z)*(posB.z-posA.z);
			if(length <= (5+5)*(2+2))
			{
				playerBullet->OnCollision();
				enemyBullet->OnCollision();
			}
		}
	}

#pragma endregion
}
