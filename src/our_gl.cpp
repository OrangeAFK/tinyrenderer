#include "our_gl.h"
#include <iostream>
mat<4,4> ModelView;
mat<4,4> Viewport;
mat<4,4> Projection;
const float depth = 255;

void viewport(int x, int y, int w, int h) {
    // transforms bi-unit cube [-1,1]^3 to cube [x,x+w]*[y,y+h]*[0,depth]
    Viewport = mat<4, 4>::identity();
    // scaling (ex. [-1,1] -> [-w/2, w/2])
    Viewport[0][0] = w/2.f;
    Viewport[1][1] = h/2.f;
    Viewport[2][2] = depth/2.f;
    // translation (ex. [-w/2, w/2] -> [x, x+w])
    Viewport[0][3] = x+w/2.f;
    Viewport[1][3] = y+h/2.f;
    Viewport[2][3] = depth/2.f;
}

void projection(const double f) {
    /*Projection = {{ {1,0,0,0}, {0,-1,0,0}, {0,0,1,0}, {0,0,f,0} }};*/
    Projection = mat<4,4>::identity();
    Projection[3][2] = f;
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    /*
    vec3 z = (center - eye).normalized();
    vec3 x = cross(up, z).normalized();
    vec3 y = cross(z, x).normalized();

    mat<4, 4> Minv = mat<4,4>::identity();
    mat<4, 4> Tr = mat<4,4>::identity();

    // note that M^-1 (Minv) = M^T since the column vectors
    // of M span an orthonormal basis
    Minv[0] = embed<4>(x, 0);
    Minv[1] = embed<4>(y, 0);
    Minv[2] = embed<4>(z, 0);

    Tr.set_col(3, embed<4>(-1*eye));

    // translate to origin and then change basis
    ModelView = Minv * Tr;
    */
    vec3 z = (eye-center).normalized();
    vec3 x = cross(up,z).normalized();
    vec3 y = cross(z,x).normalized();
    ModelView = mat<4,4>::identity();
    for (int i=0; i<3; i++) {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -center[i];
    }
}

vec3 cartesian_to_barycentric(const vec2 tri[3], const vec2 P) {
    // P = alpha*A + beta*B + gamma*C = (ABC)^T[alpha beta gamma] --> [alpha beta gamma] = ABC^T^-1(P)
    mat<3,3> ABC = {{embed<3>(tri[0]), embed<3>(tri[1]), embed<3>(tri[2])}};
    if(ABC.det()<1e-3) { return {-1,1,1}; } // throw away degenerate triangles
    return ABC.inverse_transpose() * embed<3>(P);
}

void triangle(const vec4 clip_verts[3], IShader& shader, TGAImage& image, std::vector<double>& zbuffer) {
    // pts2 are vertices on screen space
    vec4 pts[3] = {Viewport*clip_verts[0], Viewport*clip_verts[1], Viewport*clip_verts[2]}; // before perspective division
    vec2 pts2[3] =  {proj<2>(pts[0]/pts[0][3]), proj<2>(pts[1]/pts[1][3]), proj<2>(pts[2]/pts[2][3])}; // after perspective division

    // find the bounding box for the triangle
    int bboxmin[2] = {image.width()-1, image.height()-1};
    int bboxmax[2] = {0, 0};
    for(int i=0; i<3; i++) {
        for(int j=0; j<2; j++) {
            bboxmin[j] = std::min(bboxmin[j], static_cast<int>(pts2[i][j]));
            bboxmax[j] = std::max(bboxmax[j], static_cast<int>(pts2[i][j]));
        }
    }

    // for each pixel in the bounding box
    for(int x=std::max(bboxmin[0], 0); x<=std::min(bboxmax[0], image.width()-1); x++) {
        for(int y=std::max(bboxmin[1], 0); y<=std::min(bboxmax[1], image.height()-1); y++) {
            vec3 bc_screen = cartesian_to_barycentric(pts2, {static_cast<double>(x), static_cast<double>(y)});
            vec3 bc_clip = {bc_screen.x/pts[0][3], bc_screen.y/pts[1][3], bc_screen.z/pts[2][3]};
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
            double frag_depth = bc_clip * vec3{clip_verts[0][2], clip_verts[1][2], clip_verts[2][2]};
            if(bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0 || zbuffer[x+y*image.width()] > frag_depth) continue;
            TGAColor color;
            if(shader.fragment(bc_clip, color)) continue;
            zbuffer[x+y*image.width()] = frag_depth;
            image.set(x, y, color);
        }
    }
}