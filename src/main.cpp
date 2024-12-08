#include <vector>
#include <cmath>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 800;
const int height = 500;

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

void rasterize(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color, int ybuffer[]) {
    // assume p1.x >= p0.x
    if(p1.x < p0.x) { std::swap(p1, p0); }
    for(int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = (p1.y - p0.y)*t + p0.y + 0.5;
        if(ybuffer[x] < y) {
            ybuffer[x] = y;
            image.set(x, 0, color);
        }
    }

}

int main() {
    { // dumping the 2d scene
        TGAImage scene(width, height, TGAImage::RGB);
        line(Vec2i(20, 34),   Vec2i(744, 400), scene, red);
        line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
        line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

        // screen line
        line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

        scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        scene.write_tga_file("scene.tga");
    }
    { // y-slice viewed from above of the 2d scene
        TGAImage render(width, 16, TGAImage::RGB);
        int ybuffer[width];
        for(int i=0; i<width; i++) { ybuffer[i] = std::numeric_limits<int>::min(); }
        rasterize(Vec2i(20, 34),   Vec2i(744, 400), render, red,   ybuffer);
        rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
        rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue,  ybuffer);

        // widen the thickness of the 1-pixel image
        for(int i=0; i<width; i++) {
            for(int j=1; j<16; j++) {
                render.set(i, j, render.get(i, 0));
            }
        }

        render.flip_vertically();
        render.write_tga_file("render.tga");
    }
	return 0;
}