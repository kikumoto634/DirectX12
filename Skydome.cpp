#include "Skydome.h"
#include <cassert>

using namespace std;

void Skydome::Initialize(Model* model, Object3D* modelObject, Vector3 pos)
{
	assert(model);

	this->model = model;
	this->modelObject = modelObject;
	this->position = pos;

	this->modelObject->Initialize();
	this->modelObject->SetModel(this->model);
	this->modelObject->SetPosition(this->position);
}

void Skydome::Update()
{
	modelObject->Update();
}

void Skydome::Draw(ID3D12GraphicsCommandList *commandList)
{
	modelObject->Draw(commandList);
}
