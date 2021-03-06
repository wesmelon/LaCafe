////////////////////////////////////////
// PlayState.cpp
////////////////////////////////////////

#include "PlayState.h"
#include "MeshLoader.h"
#include "StateManager.h"
#include "TimeManager.h"
#include <cstdio>
#include "Globals.h"

#define Y_OFFSET 0.5

////////////////////////////////////////////////////////////////////////////////

PlayState::PlayState(GLFWwindow* window, Restaurant* area) : GameState(window) {
	srand(time(NULL));

	a = area;
	a->setTile(2, 2, RECEPTION);
	a->setTile(2, 3, RECEPTION_WORKER);
	a->setTile(5, 0, TABLE_CHAIR);
	a->setTile(5, 2, TABLE_CHAIR);
	a->setTile(5, 4, TABLE_CHAIR);
	a->setTile(6, 0, TABLE);
	a->setTile(6, 2, TABLE);
	a->setTile(6, 4, TABLE);
	a->setTile(2, 5, STOVE);
	a->setTile(2, 6, STOVE_WORKER);
	a->setTile(4, 5, STOVE);
	a->setTile(4, 6, STOVE_WORKER);

	a->fillPaths();
	Initialize();
}

////////////////////////////////////////////////////////////////////////////////

int PlayState::Initialize() {
	glEnable(GL_DEPTH_TEST);

	MouseActiveButton = 0;
	
	Skybox.Initialize();
	std::string cmRelPath = "CubeMap/Yokohama3/";
	Skybox.CreateCubeMap((cmRelPath + "negz.jpg").c_str(),  // front
						 (cmRelPath + "posz.jpg").c_str(),  // back
						 (cmRelPath + "posy.jpg").c_str(),  // top
						 (cmRelPath + "negy.jpg").c_str(),  // bottom
						 (cmRelPath + "negx.jpg").c_str(),  // left
						 (cmRelPath + "posx.jpg").c_str()); // right
	g_Floor.SetPosition(vec3(0, 0, 0));

	g_Axis.Initialize(Model::axis, 6, GL_LINES, "Shaders/Shader_vs.glsl", "Shaders/Shader_fs.glsl");
	g_Axis.SetPosition(vec3(0, 0, 0));
	
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front
	glClearColor(1.0, 1.0, 1.0, 1.0); // grey background to help spot mistakes

	g_SquarePath = new std::vector<Model>(NUM_OF_SQUARES);
	for (int i = 0; i < NUM_OF_SQUARES; i++) {
		g_SquarePath->at(i).Initialize(Model::square2, 6, GL_TRIANGLES, "Shaders/Shader_vs.glsl", "Shaders/Shader_fs.glsl");
		g_SquarePath->at(i).SetPosition(vec3(i / 10, Y_OFFSET, i % 10));
		g_SquarePath->at(i).SetScale(vec3(0.5, 0.5, 0.5));
	}

	// Fill object vectors
	Resume();

	PersonModel = new Mesh("Models/Dude.fbx", "Shaders/Banana_vs.glsl", "Shaders/Banana_fs.glsl");
	g_Receptionist = Employee(a, RECEPTIONIST, PersonModel);

	g_Employees.push_back(new Employee(a, COOK, PersonModel));
	g_Employees.push_back(new Employee(a, COOK, PersonModel));
	g_Employees.push_back(new Employee(a, WAITER, PersonModel));
	g_Employees.push_back(new Employee(a, DISHWASHER, PersonModel));

	return INIT_OK; // OK
}

void PlayState::Resume() {
	StateRunning = true;

	// Deallocate memory first
	for (Patron *p : g_Patron) {
		delete p;
	}
	g_Patron.clear();

	a->fillPaths();
}

////////////////////////////////////////////////////////////////////////////////

void PlayState::Input() {
	// Camera output
	/*fprintf(stdout, "Camera POSITION: (%.2f, %.2f, %.2f), YAW: %.2f, PITCH: %.2f\n",
		Globals::Camera.GetPosition().x,
		Globals::Camera.GetPosition().y,
		Globals::Camera.GetPosition().z, 
		Globals::Camera.GetYaw(),
		Globals::Camera.GetPitch());*/
	// Keyboard
	if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (glfwGetKey(window, GLFW_KEY_W)) {
		Globals::Camera.MoveForward(Globals::Camera.GetSpeed());
	}
	if (glfwGetKey(window, GLFW_KEY_A)) {
		Globals::Camera.MoveLeft(Globals::Camera.GetSpeed());
	}
	if (glfwGetKey(window, GLFW_KEY_S)) {
		Globals::Camera.MoveBackward(Globals::Camera.GetSpeed());
	}
	if (glfwGetKey(window, GLFW_KEY_D)) {
		Globals::Camera.MoveRight(Globals::Camera.GetSpeed());
	}
	if (glfwGetKey(window, GLFW_KEY_R)) {
		Globals::Camera.MoveUp(Globals::Camera.GetSpeed());
	}
	if (glfwGetKey(window, GLFW_KEY_F)) {
		Globals::Camera.MoveDown(Globals::Camera.GetSpeed());
	}
	if (glfwGetKey(window, GLFW_KEY_UP)) {
		Globals::Camera.SetPitch(Globals::Camera.GetPitch() + Globals::Camera.GetSpeed() * -1/1000);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN)) {
		Globals::Camera.SetPitch(Globals::Camera.GetPitch() + Globals::Camera.GetSpeed() * 1/1000);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT)) {
		Globals::Camera.SetYaw(Globals::Camera.GetYaw() + Globals::Camera.GetSpeed() * -1/1000);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
		Globals::Camera.SetYaw(Globals::Camera.GetYaw() + Globals::Camera.GetSpeed() * 1/1000);
	}

	// Mouse motions

	/* Find mouse displacement
	MouseX = nx;
	MouseY = ny;
	double dx = nx - MouseX;
	double dy = ny - MouseY;*/

	static double clickStartTime = TimeManager::Instance().GetTime();

	// Raycast
	if (MouseActiveButton & MOUSE_LEFT) {
		if (TimeManager::Instance().CurrentTime - clickStartTime > 1.0f) {
			clickStartTime = TimeManager::Instance().CurrentTime;
			double nx, ny;

			glfwGetCursorPos(window, &nx, &ny);

			vec3 ray_wor = GetRayFromMouse((float)nx, (float)ny, WinX, WinY);

			int closest_square_clicked = -1;
			float closest_intersection = 0.0f;
			/*for (int i = 0; i < NUM_OF_SQUARES; i++) {
				float t_dist = 0.0f;
				if (RayIntersect(Globals::Camera.GetPosition(), ray_wor, g_SquarePath->at(i).GetPosition(), SquareRadius, &t_dist)) {
					// if more than one sphere is in path of ray, only use the closest one
					if (-1 == closest_square_clicked || t_dist < closest_intersection) {
						closest_square_clicked = i;
						closest_intersection = t_dist;

						// Clear the previous paths
						for (int k = 0; k < a->getHeight(); k++)
							for (int l = 0; l < a->getWidth(); l++) {
								if (a->getTileType(k, l) == TABLE_CHAIR)
									a->setTile(k, l, WALKABLE);
							g_SquarePath->at(l + a->getHeight() * k).Unpath();
							}

						// To only allow one destination
						if (a->getTileType(closest_square_clicked / 10, closest_square_clicked % 10) == WALKABLE)
							a->setTile(closest_square_clicked / 10, closest_square_clicked % 10, TABLE_CHAIR);

						a->fillPaths();

						Cell* c = nullptr;
						for (int z = 0; z < a->getHeight(); z++)
							for (int x = 0; x < a->getWidth(); x++)
								if (a->getTileType(z, x) == START) {
							c = new Cell;
							c->x = x;
							c->z = z;
								}

						if (c != nullptr) {
							for (int z = 0; z < a->getHeight(); z++)
								for (int x = 0; x < a->getWidth(); x++) {
									if (a->getTileType(z, x) == RECEPTION) {
									// If the cell is a potential destination, print the path
									deque<Cell*> p = a->getCellPath(c->z, c->x, z, x);
									for (size_t j = 0; j < p.size(); j++) {
										Cell* top = p.at(j);
										// 7 is just a visual symbol to represent the path
										//pathMap.at(top->x + a.getHeight() * top->z) = 7;
										g_SquarePath->at(top->x + a->getHeight() * top->z).Path();
									}
								}
								}
							delete c;
						}
					}
				}
			}
			SelectedSquare = closest_square_clicked;*/
			if (a->podiumReachable())
				g_Patron.push_back(new Patron(a, PersonModel));
			//fprintf(stdout, "center of square: x: %.2f, z: %.2f", g_SquarePath->at(SelectedSquare).GetPosition().x, g_SquarePath->at(SelectedSquare).GetPosition().z);
		}
	}

	glfwPollEvents();
}

////////////////////////////////////////////////////////////////////////////////

void PlayState::Update() { 
	for (unsigned int i = 0; i < g_Patron.size(); i++) {
		if (g_Patron.at(i)->canDelete()) {
			// Delete the patron now that it's done eating/waiting
			delete g_Patron.at(i);
			g_Patron.erase(g_Patron.begin() + i);
		}
		else {
			g_Patron.at(i)->update();
		}
	}

	for (unsigned int i = 0; i < g_Employees.size(); i++)
		g_Employees.at(i)->update();
	g_Receptionist.update();

	// Spawn a patron randomly: at popularity 100, spawn every 5 seconds (on average)
	// Assume 100 fps (DT = 10) => 500 frames on average, assume 50 fps (DT = 20) => 250
	// 1 fps => denominator = 500000, 2fps => denominator = 1000000
	// Spawn rate = 1/5000 = 100 / 5000 * fps
	double frames = TimeManager::Instance().CalculateFrameRate(false);
	if (frames < 50) return;
	int spawnPatronChance = rand() % 5000;
	if(a->m_popularity > (spawnPatronChance * frames / 5))
		if (a->podiumReachable())
			g_Patron.push_back(new Patron(a, PersonModel));
}

////////////////////////////////////////////////////////////////////////////////

void PlayState::Draw() {
	//Skybox.Render();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	g_Receptionist.Render();

	g_Floor.Render();
	for (Employee *e : g_Employees) {
		e->Render();
	}
	
	for (Patron *p : g_Patron) {
		p->Render();
	}

	for (GameObject *g : a->g_GameObjects) {
		if (g != NULL)
			g->Render();
	}

	g_Axis.Render();

	for (int i = 0; i < NUM_OF_SQUARES; i++) {
		if (SelectedSquare == i) {
			g_SquarePath->at(i).Select();
		}
		else {
			g_SquarePath->at(i).Unselect();
		}
		g_SquarePath->at(i).Render();
	}
}

////////////////////////////////////////////////////////////////////////////////