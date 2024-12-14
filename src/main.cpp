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
const int height = 800;
const int depth = 255;

Vec3f light_dir(0,0,-1);
Vec3f camera(0,0,3);

Matrix vector_to_matrix(Vec3f& v) {
    Matrix m(4, 1);
    m[0][0] = v[0];
    m[1][0] = v[1];
    m[2][0] = v[2];
    m[3][0] = 1.f;
    return m;
}

Vec3f matrix_to_vector(Matrix& m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

Matrix viewport(int x, int y, int w, int h) {
    // transforms bi-unit cube [-1,1]^3 to cube [x,x+w]*[y,y+h]*[0,depth]
    Matrix m = Matrix::identity(4);
    // scaling (ex. [-1,1] -> [-w/2, w/2])
    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    // translation (ex. [-w/2, w/2] -> [x, x+w])
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;
    return m;
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2. + .5), int((v.y+1.)*height/2. + .5), v.z);
}

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

Vec3f cartesian_to_barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    // find two edges and the distance from P to the shared vertex
    // consider only x- and y- coords (z-buffer will be considered separately)
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = s[0] ^ s[1];
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
    // find the appropriate bounding box
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for(int i=0; i<3; i++) {
        for(int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for(P.x = bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for(P.y = bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bary = cartesian_to_barycentric(pts[0], pts[1], pts[2], P);
            if (bary.x<0 || bary.y<0 || bary.z<0) { continue; } // out of bounds
            // z-buffer
            P.z = 0;
            for(int i=0; i<3; i++) { P.z += pts[i][2]*bary[i]; }
            if(zbuffer[int(P.x + P.y*width)] < P.z) {
                zbuffer[int(P.x + P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
            
        }
    }
}

void texturedTriangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAImage& texData,Vec3f* texture) {
    // TODO : make this more modular
    int texWidth=texData.get_width(), texHeight=texData.get_height();
    TGAColor color0(texData.get(texture[0][0]*texWidth, texture[0][1]*texHeight));
    TGAColor color1(texData.get(texture[1][0]*texWidth, texture[1][1]*texHeight));
    TGAColor color2(texData.get(texture[2][0]*texWidth, texture[2][1]*texHeight));
    // find the appropriate bounding box
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for(int i=0; i<3; i++) {
        for(int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for(P.x = bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for(P.y = bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bary = cartesian_to_barycentric(pts[0], pts[1], pts[2], P);
            if (bary.x<0 || bary.y<0 || bary.z<0) { continue; } // out of bounds
            // z-buffer
            P.z = 0;
            for(int i=0; i<3; i++) { P.z += pts[i][2]*bary[i]; }
            if(zbuffer[int(P.x + P.y*width)] < P.z) {
                zbuffer[int(P.x + P.y*width)] = P.z;
                Vec3f texCoord = texture[0]*bary.x + texture[1]*bary.y + texture[2]*bary.z;
                TGAColor color(texData.get(texCoord[0]*texWidth, texCoord[1]*texHeight));
                image.set(P.x, P.y, color);
            }
            
        }
    }
}

int main(int argc, char** argv) {
    Model* model;
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    float *zbuffer = new float[width*height];
    for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());
    Matrix Proj = Matrix::identity(4);
    Proj[3][2] = -1.f/camera.z;
    Matrix Viewport = viewport(width/8, height/8, width*3/4, height*3/4);

    TGAImage image(width, height, TGAImage::RGB);

    TGAImage texData;
    texData.read_tga_file("obj/african_head_diffuse.tga");
    texData.flip_vertically();

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<Vec3i> face = model->face(i);
        Vec3f screen[3];
        Vec3f world[3];
        Vec3f texture[3];
        for (int i=0; i<3; i++) {
            world[i] = model->vert(face[i][0]);
            //std::cout << 'w' << world[i];
            screen[i] = matrix_to_vector(Viewport*Proj*vector_to_matrix(world[i]));
            //std::cout << 's' << screen[i];
            //screen[i] = world2screen(world[i]);
            texture[i] = model->texture(face[i][1]);
        }
        Vec3f n = (world[2]-world[0]) ^ (world[1]-world[0]);
        n.normalize();
        float intensity = n * light_dir;
        //triangle(screen, zbuffer, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        texturedTriangle(screen, zbuffer, image, texData, texture);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}