#pragma once

#include "Framework/Scenes/Scene.h"

#include "NoNameGestor/Gestor/Gestor.h"

struct SDL_Renderer;

class EconomyScene : public Scene
{
public:

	bool Awake() override;
	bool Start() override;
	bool Update(float dt) override;
	bool Draw(float dt) override;
	bool CleanUp() override;

private: // Variables

	SDL_Renderer* renderer = nullptr;
	Gestor gestor;

};
