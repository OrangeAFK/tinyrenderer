#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
protected:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<Vec3i> > faces_;
	std::vector<Vec3f> textures_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int ntextures();
	Vec3f vert(int i);
	std::vector<Vec3i> face(int idx);
	Vec3f texture(int i);
};

#endif //__MODEL_H__