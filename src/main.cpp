#include <vector>
#include <cmath>
#include <iostream>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const int width = 800;
const int height = 800;

void line(Vec2i& p0, Vec2i& p1, TGAImage& image, TGAColor color) { 
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

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    line(t0, t1, image, color);
    line(t1, t2, image, color);
    line(t2, t0, image, color);
}

int main(int argc, char** argv) {
    // wireframe render
    Model* model = nullptr;
    model = (argc==2) ? new Model(argv[1]): new Model("obj/african_head.obj");
	TGAImage image(width, height, TGAImage::RGB);
    
    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); // set origin to bottom-left of the screen
	image.write_tga_file("output.tga");
    delete model;
	return 0;
}