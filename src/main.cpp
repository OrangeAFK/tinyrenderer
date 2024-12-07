#include <vector>
#include <cmath>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const int width = 200;
const int height = 200;

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color) { 
    bool steep = false;
    if(std::abs(p1.x-p0.x) < std::abs(p1.y-p0.y)) // transpose iterator if the line is steep
    {
        steep = true;
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
    }
    if(p0.x > p1.x) // render left-to-right
    {
        std::swap(p0.x, p1.x);
        std::swap(p0.y, p1.y);
    }
    // slope of line after transpose is guaranteed to be in -1<=m<=1
    // will change y by 1 when the accumulated error crosses a threshold
    // this enables precomputing derror, eliminating floating points and division
    int dx = p1.x-p0.x;
    int dy = p1.y-p0.y;
    int derror2 = 2 * std::abs(dy); // doubled so that the error threshold doubles to 1
    int error2 = 0;

    int y = p0.y;
    for (int x=p0.x; x<=p1.x; x++) { 
        if(steep)
        {
            image.set(y, x, color);  // de-transpose line if transposed
        }
        else
        {
            image.set(x, y, color);
        }
        error2 += derror2;
        if(error2 > dx)
        {
            y += (p1.y>p0.y ? 1 : -1);
            error2 -= 2 * dx;
        }
    } 
}

Vec3f cartesian_to_barycentric(Vec2i* pts, Vec2i point) {
    // get two triangle edges and a vector from a vertex to the point
    Vec2i v0 = pts[1] - pts[0];
    Vec2i v1 = pts[2] - pts[0];
    Vec2i p = point - pts[0];

    int triangleCross = v1.x*v0.y - v0.x*v1.y;
    
    float alpha = (p.x*v0.y - v0.x*p.y)/(float)triangleCross;
    float beta = (v1.x*p.y - p.x*v1.y)/(float)triangleCross;
    float gamma = 1 - alpha - beta;
    return Vec3f(alpha, beta, gamma);
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    if (t0.y==t1.y && t0.y==t2.y) return; // i dont care about degenerate triangles
    if (t0.y>t1.y) std::swap(t0, t1);
    if (t0.y>t2.y) std::swap(t0, t2);
    if (t1.y>t2.y) std::swap(t1, t2);
    int total_height = t2.y-t0.y;
    for (int i=0; i<total_height; i++) {
        bool second_half = i>t1.y-t0.y || t1.y==t0.y;
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y;
        float alpha = (float)i/total_height;
        float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here
        Vec2i A =               t0 + (t2-t0)*alpha;
        Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta;
        if (A.x>B.x) std::swap(A, B);
        for (int j=A.x; j<=B.x; j++) {
            image.set(j, t0.y+i, color); // attention, due to int casts t0.y+i != A.y
        }
    }
}

int main() {
    Model* model = new Model("obj/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);

Vec3f light_dir(0,0,-1); // define light_dir

for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i); 
    Vec2i screen_coords[3]; 
    Vec3f world_coords[3]; 
    for (int j=0; j<3; j++) { 
        Vec3f v = model->vert(face[j]); 
        screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.); 
        world_coords[j]  = v; 
    } 
    Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]); 
    n.normalize(); 
    float intensity = n*light_dir; 
    if (intensity>0) { 
        triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255)); 
    } 
}

	image.flip_vertically(); // set origin to bottom-left of the screen
	image.write_tga_file("output.tga");
	return 0;
}