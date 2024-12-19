#ifndef OUR_GL_H
#define OUR_GL_H
#include "tgaimage.h"
#include "geometry.h"

extern mat<4, 4> ModelView, Projection, Viewport;

void viewport(const int, const int, const int, const int);
void projection(double coeff=0); // coeff = -1/c
void lookat(const vec3 eye, const vec3 center, const vec3 up);

struct IShader {
    virtual vec4 vertex(int iface, int nthvert) = 0;
    virtual bool fragment(vec3 bar, TGAColor& color) = 0;
};

void triangle(const vec4 clip_verts[3], IShader& shader, TGAImage& image, std::vector<double>& zbuffer);

#endif
