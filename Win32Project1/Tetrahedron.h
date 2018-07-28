#pragma once
class Tetrahedron
{
private:
	GLfloat red;
	GLfloat green;
	GLfloat blue;
	GLfloat* center;

public:
	GLint faces[4][3];// = { {0, 1, 2}, {0, 3, 1}, {0, 2, 3}, {1, 3, 2} };
	GLfloat v[4][3];// = { {-1, 0.7071, 0},	{0, -0.7071, -1}, {1, 0.7071, 0}, {0, -0.7071, 1} };


	Tetrahedron();
	~Tetrahedron();
	void draw(float);
	Tetrahedron* createTetrohedron(int);
	void setColor(GLfloat, GLfloat, GLfloat);
	GLfloat* getCenter();
	void translate(GLfloat*);
	void setAlpha(float);
};

