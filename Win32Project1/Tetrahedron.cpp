#include "stdafx.h"
#include "Tetrahedron.h"

//GLint faces2[4][3] = { {0, 1, 2}, {0, 3, 1}, {0, 2, 3}, {1, 3, 2} };
//
//GLfloat v2[4][3] = {
//	{-1, 0.7071, 0},
//	{0, -0.7071, -1},
//	{1, 0.7071, 0},
//	{0, -0.7071, 1} };

Tetrahedron::Tetrahedron(void)
{
	faces[0][0] = 0;
	faces[0][1] = 1;
	faces[0][2] = 2;
	faces[1][0] = 0;
	faces[1][1] = 3;
	faces[1][2] = 1;
	faces[2][0] = 0;
	faces[2][1] = 2;
	faces[2][2] = 3;
	faces[3][0] = 1;
	faces[3][1] = 3;
	faces[3][2] = 2;

	v[0][0] = -1;
	v[0][1] = 1.0f/sqrt(2.0f);    //0.7071;
	v[0][2] = 0;
	v[1][0] = 0;
	v[1][1] = -1.0f/sqrt(2.0f);   //-0.7071;
	v[1][2] = -1;
	v[2][0] = 1;
	v[2][1] = 1.0f/sqrt(2.0f);    //0.7071;
	v[2][2] = 0;
	v[3][0] = 0;
	v[3][1] = -1.0f/sqrt(2.0f);    //-0.7071;
	v[3][2] = 1;

	red = 1.0;
	green = 0.0;
	blue = 0.0;
	center = NULL;
}

void Tetrahedron::draw(float alpha) {
	int i;
	for(i=0; i!= 4; ++i){
		glBegin(GL_TRIANGLES);
		glColor4f(red, green, blue, alpha);
		glVertex3fv(&v[faces[i][0]][0]);
		glVertex3fv(&v[faces[i][1]][0]);
		glVertex3fv(&v[faces[i][2]][0]);
		glEnd();
	}
}

/*
	created tetrahedron should be deleted outside
*/
Tetrahedron* Tetrahedron::createTetrohedron(int k){
	Tetrahedron* result = new Tetrahedron();

	//count sum except v[k]
	GLfloat sum[3] = {0.0, 0.0, 0.0};
	for(int i=0; i!=4; ++i){
		for(int j=0; j!=3; ++j){
			if(i!=k){
				sum[j] += v[i][j];
			}
		}
	}

	// count new coordinates
	for(int i=0; i!=4; ++i){
		for(int j=0; j!=3; ++j){
			if(i==k){
				result->v[i][j] = v[i][j] + 2.0f*((1.0f/3.0f)*sum[j]-v[i][j]);
			}else{
				result->v[i][j] = v[i][j];
			}
		}
	}

	return result;
}

void Tetrahedron::setColor(GLfloat red, GLfloat green, GLfloat blue){
	this->red = red;
	this->green = green;
	this->blue = blue;
}

GLfloat* Tetrahedron::getCenter(){
	if(center==0){
		center = new GLfloat[3];

		for(int i=0; i!=3; ++i){
			center[i] = 0.0;
			for(int j=0; j!=4; ++j){
				center[i] += v[j][i];
			}
			center[i] /= 4.0;
		}
	}

	return center;
}

void Tetrahedron::translate(GLfloat* d){
	for(int i=0; i!=4; ++i){
		for(int j=0; j!=3; ++j){
			v[i][j] -= d[j];
		}
	}

	for(int j=0; j!=3; ++j){
		center[j] -= d[j];
	}
}

Tetrahedron::~Tetrahedron(){
	if(!center) delete center;
}
