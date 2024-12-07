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

void triangle(Vec2i* pts, TGAImage &image, TGAColor color) {
    // get bounding box
    Vec2i bboxMin(image.get_width()-1, image.get_height()-1);
    Vec2i bboxMax(0, 0);
    Vec2i screenMax(image.get_width()-1, image.get_height()-1);
    for(int i=0; i<3; i++)
    {
        bboxMin.x = std::max(0, std::min(bboxMin.x, pts[i].x));
        bboxMax.x = std::min(screenMax.x, std::max(bboxMax.x, pts[i].x));

        bboxMin.y = std::max(0, std::min(bboxMin.y, pts[i].y));
        bboxMax.y = std::min(screenMax.y, std::max(bboxMax.y, pts[i].y));
    }
    Vec2i P;
    for(P.x=bboxMin.x; P.x<=bboxMax.x; P.x++)
    {
        for(P.y=bboxMin.y; P.y<=bboxMax.y; P.y++)
        {
            Vec3f baryCoords = cartesian_to_barycentric(pts, P);
            if(baryCoords.x<0 || baryCoords.y<0 || baryCoords.z<0) { continue; }
            image.set(P.x, P.y, color);
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
        triangle(screen_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255)); 
    } 
}

	image.flip_vertically(); // set origin to bottom-left of the screen
	image.write_tga_file("output.tga");
	return 0;
}