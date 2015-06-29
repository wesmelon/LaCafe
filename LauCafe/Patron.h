#ifndef PATRON_H
#define PATRON_H

#include "MeshLoader.h"
#include "Area.h"
#include "Person.h"

class Patron : public Person
{
public:
	Patron() {}
	Patron(Area* area, Camera *m_Camera);
	~Patron();

	void walkToCells();
	void findNextDestination();
	void finishCurrentTask();

	void Render();
private:
	Mesh dude;
	int pathIndex;
};

#endif