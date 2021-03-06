#pragma once

#include <vector>
#include <deque>
#include "GameObjectTable.h"
#include <unordered_map>

using namespace std;

enum TileType { WALKABLE, START, OBSTACLE, RECEPTION, RECEPTION_WORKER, TABLE, TABLE_CHAIR, STOVE,
	STOVE_WORKER, BAR, BAR_WORKER, TOILET, INVALID_TYPE};
enum TileStatus {
	TILE_OPEN, TILE_RESERVED,
	TILE_STOVE_COOKING, TILE_STOVE_FOOD,
	TILE_TABLE_FOOD_COMING, TILE_TABLE_FOOD, TILE_TABLE_DIRTY,
	TILE_INVALID_STATUS
};
enum ReceptionistStatus { REC_READY, REC_JUST_DIRECTED, REC_COOLDOWN };

// Represents a single tile in the array
struct Cell {
	int x;
	int z;
};

struct MeshAndCounter {
	Mesh* mesh;
	int count;
};

class Restaurant
{
public:
	/*
	* The convention used in the function parameters is:
	* z: the vertical coordinate
	* x: the horizontal coordinate
	* The matrices used to represent the floor and paths are technically 1D
	*   vectors, but they are accessed using the getIndex(z,x) function
	* sz: the vertical coordinate of the start/origin cell
	* sx: the horizontal coordinate of the start/origin cell
	* dz: the vertical coordinate of the destination cell
	* dx: the horizontal coordinate of the destination cell
	*/

	Restaurant() {}; // Used for avoiding a compile-time error
	Restaurant(int height, int width, int sz, int sx);
	Restaurant(int height, int width, int sz, int sx, vector<TileType> existingVector);
	~Restaurant();

	// Accessors
	Cell getStart() { return m_start; }
	int getHeight() { return m_height; }
	int getWidth() { return m_width; }
	Mesh* getMesh(const char* m);

	TileType getTileType(int z, int x);
	TileType getDestinationType(TileType);
	int getCellPathLength(int sz, int sx, int dz, int dx);
	deque<Cell*> getCellPath(int sz, int sx, int dz, int dx);
	Cell getAdjacentTable(int z, int x);
	Cell getAdjacentChair(int z, int x);
	Cell getAdjacentStove(int z, int x);

	bool isInBounds(int z, int x);
	bool isWalkable(int z, int x);
	bool podiumReachable() { return canReachPodium; }
	TileStatus getTileStatus(int z, int x);

	// Mutators
	void setTile(int z, int x, TileType tileType);
	void setTileStatus(int z, int x, TileStatus status);
	void fillPaths();
	void clearPaths();
	void decrementMesh(const char* m);

	void decrementPopularity() { if (m_popularity > 50) m_popularity--; }

	// Debugging info
	void printArray();
	void printPaths();

	// Would use queues, but what if an employee cannot reach, while another can?
	deque<Cell> v_waitingCustomerCells;
	deque<Cell> v_servingCustomerCells;
	deque<Cell> v_doneCookingStoveCells;
	deque<Cell> v_dirtyTableCells;

	vector<GameObject*> g_GameObjects;

	ReceptionistStatus recStatus;

	// Level and statistics, move to private when done testing
	int m_level = 1;
	int m_popularity = 100;		// 1-500, represented by stars (1.0-5.0, ones digit hidden)
	double m_multiplier = 1;	// Multiplies money and exp, affected by pop and decorations
	double m_money = 0;
	int m_exp = 0;
private:
	// This function is private to prevent accessing the right cell easily
	int getIndex(int z, int x);
	void fillPathLength();

	Cell m_start;
	int m_width;
	int m_height;

	// To prevent patrons from throwing exceptions
	bool canReachPodium;

	// Dynamic arrays for expandable restaurants
	vector<TileType> v_typeVector;				// Represents the actual floor of the restaurant
	vector<vector<int>> v_pathLengthVector;		// Represents the path lengths to the appropriate destinations
	vector<vector<deque<Cell*>>> v_pathVector;	// Holds the shortest distance path for tables only
	vector<TileStatus> v_statusVector;			// Tells the state of the tile
	vector<int> v_decorationVector;				// Represents what decoration occupies the table (IDs)

	unordered_map<const char*, MeshAndCounter*> m_modelMap;

	Mesh *ChairModel = new Mesh("Models/chair/chair.obj", "Shaders/Banana_vs.glsl", "Shaders/Banana_fs.glsl");
	Mesh *PodiumModel = new Mesh("Models/Podium.fbx", "Shaders/Banana_vs.glsl", "Shaders/Banana_fs.glsl");
	Mesh *StoveModel = new Mesh("Models/stove.fbx", "Shaders/Banana_vs.glsl", "Shaders/Banana_fs.glsl");
};