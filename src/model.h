#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model {
public: 
	Model(const std::string filename);
	int nverts() const;
	int nfaces() const;
	vec3 normal(const vec2&) const; 
	vec3 normal(const int, const int) const;
	vec3 vert(const int) const;
	vec3 vert(const int, const int) const;
	vec2 uv(const int, const int) const;
	const TGAImage& diffuse() const { return diffusemap; }
	const TGAImage& specular() const { return specularmap; }
private:
	std::vector<vec3> verts; // array of vertices
	std::vector<vec3> norms; // per-vertex array of normal vectors
	std::vector<vec2> tex_coord; // per-vertex array of texcoords
	std::vector<int> facet_vert, facet_tex, facet_norm; // per-triangle indices in the above arrays
	TGAImage normalmap;
	TGAImage diffusemap;
	TGAImage specularmap;
	void load_texture(const std::string, const std::string, TGAImage&);
};

#endif //__MODEL_H__