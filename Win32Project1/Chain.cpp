#include "stdafx.h"
#include "Chain.h"


Chain::Chain(void)
{
	Tetrahedron* t = new Tetrahedron();
	t->setColor(1,0,0);
	chain.push_back(t);

	addTetrahedron(0);
	addTetrahedron(1);

}

void Chain::draw(float alpha){
	for(int i=0; i!=chain.size(); ++i){
		chain[i]->draw(alpha);
	}
}

void Chain::centerize(){
	GLfloat* min = new GLfloat[3];
	GLfloat* max = new GLfloat[3];
	GLfloat* center = new GLfloat[3];

	for(int j=0; j!=3; ++j){
		min[j] = 9999;
		max[j] = -9999;
	}

	for(int i=0; i!=chain.size(); ++i){
		for(int j=0; j!=3; ++j){
			GLfloat* tetrahedronCenter = chain[i]->getCenter();
			if(tetrahedronCenter[j] > max[j]){
				max[j] = tetrahedronCenter[j];
			}
			if(tetrahedronCenter[j] < min[j]){
				min[j] = tetrahedronCenter[j];
			}
		}
	}
	for(int j=0; j!=3; ++j){
		center[j] = (min[j] + max[j])/2.0f;
	}
	/*
	GLfloat* center = new GLfloat[3];

	center[0] = center[1] = center[2] = 0;

	for(int i=0; i!=chain.size(); ++i){
		for(int j=0; j!=3; ++j){
			GLfloat* tetrahedronCenter = chain[i]->getCenter();
			center[j] += tetrahedronCenter[j];
		}
	}

	center[0] /= chain.size();
	center[1] /= chain.size();
	center[2] /= chain.size();

	*/

	for(int i=0; i!=chain.size(); ++i){
		chain[i]->translate(center);
	}
	
	delete[] center;
	delete[] min;
	delete[] max;
}

void Chain::addTetrahedron(int i){
	Tetrahedron* t = chain.at(chain.size()-1)->createTetrohedron(i);	// must be deleted
	switch (chain.size() % 3)
	{
	case 0:
		t->setColor(1,0,0);
		break;
	case 1:
		t->setColor(0,1,0);
		break;
	case 2:
		t->setColor(0,0,1);
		break;
	}
	chain.push_back(t);
	vert.push_back(i);

	centerize();
}

void Chain::addTetrahedron(char orientation){
	switch (orientation)
	{
	case 'F':
		addTetrahedron(vert[vert.size()-2]);
		break;
	default:
		Tetrahedron* last = chain.at(chain.size()-1);
		GLfloat* c3 = last->getCenter();
		int lastv = vert.at(vert.size()-1);
		Tetrahedron* prelast = chain.at(chain.size()-2);
		GLfloat* c2 = prelast->getCenter();
		Tetrahedron* preprelast = chain.at(chain.size()-3);
		GLfloat* c1 = preprelast->getCenter();
		int prelastv = vert.at(vert.size()-2);
		int nextv1;
		int nextv2;
		switch (lastv)
		{
		case 0:
			switch (prelastv)
			{
			case 1:
				nextv1 = 2;
				nextv2 = 3;
				break;
			case 2:
				nextv1 = 1;
				nextv2 = 3;
				break;
			case 3:
				nextv1 = 1;
				nextv2 = 2;
				break;
			}
			break;
		case 1:
			switch (prelastv)
			{
			case 0:
				nextv1 = 2;
				nextv2 = 3;
				break;
			case 2:
				nextv1 = 0;
				nextv2 = 3;
				break;
			case 3:
				nextv1 = 0;
				nextv2 = 2;
				break;
			}
			break;
		case 2:
			switch (prelastv)
			{
			case 0:
				nextv1 = 1;
				nextv2 = 3;
				break;
			case 1:
				nextv1 = 0;
				nextv2 = 3;
				break;
			case 3:
				nextv1 = 0;
				nextv2 = 1;
				break;
			}
			break;
		case 3:
			switch (prelastv)
			{
			case 0:
				nextv1 = 1;
				nextv2 = 2;
				break;
			case 1:
				nextv1 = 0;
				nextv2 = 2;
				break;
			case 2:
				nextv1 = 0;
				nextv2 = 1;
				break;
			}
			break;
		}
		Tetrahedron* next1 = last->createTetrohedron(nextv1);	// must be deleted
		GLfloat* c4 = next1->getCenter();
		Tetrahedron* next2 = last->createTetrohedron(nextv2);	// must be deleted
		GLfloat* c5 = next2->getCenter();
		GLfloat x1[3];
		GLfloat x2[3];
		GLfloat x3[3];
		GLfloat x4[3];

		for(int i=0; i!=3; ++i){
			x1[i] = c2[i] - c1[i];
			x2[i] = c3[i] - c2[i];
			x3[i] = c4[i] - c3[i];
			x4[i] = c5[i] - c3[i];
		}

		GLfloat d1 = determinant(x1,x2,x3);
		GLfloat d2 = determinant(x1,x2,x4);

		if(orientation=='D'){
			if(d1<0){
				addTetrahedron(nextv1);
			}else{
				addTetrahedron(nextv2);
			}
		}else if(orientation=='U'){
			if(d1>0){
				addTetrahedron(nextv1);
			}else{
				addTetrahedron(nextv2);
			}
		}

		delete next2;
		delete next1;

		break;
	}
}

void Chain::addTerrahedron(string chainCode, int repeat, string tailCode){
	for(int i=0; i!=repeat; ++i){
		for(int j=0; j!=chainCode.length(); ++j){
			addTetrahedron(chainCode[j]);
		}
	}
	for(int j=0; j!=tailCode.length(); ++j){
		addTetrahedron(tailCode[j]);
	}
	setColors(mode);
}

void Chain::deleteLast(){
	if(chain.size()>3){
		Tetrahedron* t = chain.back();
		chain.pop_back();
		delete t;
		vert.pop_back();
		centerize();
	}
}

void Chain::reset(){
	while(chain.size()>3){
		Tetrahedron* t = chain.back();
		chain.pop_back();
		delete t;
		vert.pop_back();
	}
	centerize();
}

GLfloat Chain::determinant(GLfloat* v1, GLfloat* v2, GLfloat* v3){
	return
		v1[0]*v2[1]*v3[2] +
		v2[0]*v3[1]*v1[2] +
		v3[0]*v1[1]*v2[2] -
		v3[0]*v2[1]*v1[2] -
		v2[0]*v1[1]*v3[2] -
		v1[0]*v3[1]*v2[2];

}

void Chain::setColors(int mode){
	float coef;

	switch (mode)
	{
	case 0:// solid red green blue
		this->mode = mode;
		for(int i=0; i!=chain.size(); ++i){
			switch (i%3)
			{
			case 0:
				chain.at(i)->setColor(1,0,0);
				break;
			case 1:
				chain.at(i)->setColor(0,1,0);
				break;
			case 2:
				chain.at(i)->setColor(0,0,1);
				break;
			}
		}
		break;
	case 2:// red gradient
		this->mode = mode;
		coef = 1.0f/chain.size();
		for(int i=0; i!=chain.size(); ++i){
			chain.at(i)->setColor(1, i*coef, i*coef);
		}
		break;
	case 3:// set white
		this->mode = mode;
		coef = 1.0f/chain.size();
		for(int i=0; i!=chain.size(); ++i){
			chain.at(i)->setColor(1, 1, 1);
		}
		break;
	}
}

Chain::~Chain(void)
{
	reset();
	Tetrahedron* t2 = chain[2];
	Tetrahedron* t1 = chain[1];
	Tetrahedron* t0 = chain[0];
	delete t2;
	delete t1;
	delete t0;
}
