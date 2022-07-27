#pragma once
#include "GameBase.h"
#include "Object3D.h"


class DirectXGame : public GameBase
{
public:
	static const int TextureNum = 2;
	static const int ObjectNum = 5;

public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	//スプライト
	std::unique_ptr<Sprite> sprite[TextureNum]{};

	//オブジェクト
	std::unique_ptr<GeometryObject3D> object[ObjectNum]{};

	Model* model = nullptr;
	Object3D* modelObject = nullptr;
};

