#ifndef GO_STOVE_H
#define GO_STOVE_H

#include "GameObject.h"
#include "MeshLoader.h"

class GameObjectStove : public GameObject {
public:
	GameObjectStove()									{}
	GameObjectStove(int z, int x) : GameObject(z, x)	{}
	~GameObjectStove()									{}

	void Initialize(Mesh *StoveModel);
	void Render();

private:
	Mesh *model;
};

#endif