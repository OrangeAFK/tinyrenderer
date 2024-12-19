#include <vector>
#include <cmath>
#include <iostream>

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include "our_gl.h"

constexpr int width  = 800; // output image size
constexpr int height = 800;

Model* model = nullptr;

constexpr vec3 light_dir{1,1,1}; // light source
constexpr vec3       eye{1,1,3}; // camera position
constexpr vec3    center{0,0,0}; // camera direction
constexpr vec3        up{0,1,0}; // camera up vector


struct Shader : public IShader {
    mat<2,3> varying_uv;  // same as above
    mat<4,4> uniform_M;   //  Projection*ModelView
    mat<4,4> uniform_MIT; // (Projection*ModelView).invert_transpose()

    virtual vec4 vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        vec4 gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Projection*ModelView*gl_Vertex; // transform it to screen coordinates
    }

    virtual bool fragment(vec3 bar, TGAColor &color) {
        vec2 uv = varying_uv*bar;
        vec3 n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize();
        vec3 l = proj<3>(uniform_M  *embed<4>(light_dir        )).normalize();
        vec3 r = (n*(n*l*2.f) - l).normalize();   // reflected light
        double spec = pow(std::max(r[2], 0.), model->specular(uv));
        double diff = std::max(0., n*l);
        TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<double>(5 + c[i]*(diff + .6*spec), 255);
        return false;
    }
};

int main(int argc, char** argv) {
    if(argc==2) { model = new Model(argv[1]); }
    else { model = new Model("obj/african_head.obj"); }

    // build transformation matrices
    lookat(eye, center, up); // ModelView
    viewport(width/8, height/8, width*3/4, height*3/4);
    projection(-1./(eye-center).norm());

    TGAImage image(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height, std::numeric_limits<double>::min());

    Shader shader;
    shader.uniform_M = Projection * ModelView;
    shader.uniform_MIT = (Projection * ModelView).inverse_transpose();
    int n = model->nfaces();
    for(int i=0; i<n; i++) {
        vec4 screen_coords[3];
        for(int j=0; j<3; j++) {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
        //std::cout << "\rtriangles: " << i+1 << " / " << n << std::flush;
    }

    image.write_tga_file("output.tga");
    delete model;
    return 0;
}