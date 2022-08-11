#pragma once
#include "GameBase.h"

#include "Player.h"

class DirectXGame : public GameBase
{

public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	std::unique_ptr<Player> player;
};

