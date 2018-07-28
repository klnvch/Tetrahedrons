#pragma once

#include "Tetrahedron.h"

class Chain
{
public:
	vector<Tetrahedron*> chain;
	vector<int> vert;
	int mode;

	Chain();
	~Chain();
	void draw(float);
	void centerize();
	void addTetrahedron(int);
	void addTetrahedron(char);
	void addTerrahedron(string, int, string);
	void deleteLast();
	void reset();
	GLfloat determinant(GLfloat*, GLfloat*, GLfloat*);
	void setColors(int);
};

