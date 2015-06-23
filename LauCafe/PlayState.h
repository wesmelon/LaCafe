////////////////////////////////////////
// PlayState.h
////////////////////////////////////////

#ifndef PLAYSTATE_H
#define PLAYSTATE_H

#include "Main.h"
#include "GameState.h"
#include "Camera.h"
#include "Model.h"
#include <vector>

#define LEFT        1
#define MIDDLE      2
#define RIGHT       3

class PlayState : public GameState
{
public:
	PlayState(GLFWwindow* window);
	
	int Initialize();

	void Input();
	void Update();
	void Draw();

private:
	// Window management
	Camera* m_Camera;
	float ratio;
	int WinX, WinY;

	// Input
	double MouseX, MouseY;
	int MouseActiveButton;
	
	Model g_Axis;
	Model g_SampleSquare;
	Model g_SampleSquare2;
	std::vector<Model> g_SampleSquare3;
};

////////////////////////////////////////////////////////////////////////////////

#endif