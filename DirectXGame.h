#pragma once
#include "GameBase.h"


class DirectXGame : public GameBase
{
public:
	//static const int TextureNum = 2;
	//static const int ObjectNum = 5;

public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

private:
	////スプライト
	//std::unique_ptr<Sprite> sprite[TextureNum]{};

	////オブジェクト
	//std::unique_ptr<GeometryObject3D> object[ObjectNum]{};

	//std::unique_ptr<Model> model;
	//std::unique_ptr<Object3D> modelObject;
};

