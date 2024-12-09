#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_(), textures_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i> f;
            int iF, iT, iN;
            iss >> trash;
            while (iss >> iF >> trash >> iT >> trash >> iN) {
                iF--; iT--; iN--; // in wavefront obj all indices start at 1, not zero
                f.push_back(Vec3i(iF, iT, iN));
            }
            faces_.push_back(f);
        } else if(!line.compare(0, 2, "vt")) {
            iss >> trash;
            iss >> trash;
            Vec3f vt;
            for (int i=0;i<3;i++) iss >> vt.raw[i];
            textures_.push_back(vt);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::ntextures() {
    return (int)textures_.size();
}

std::vector<Vec3i> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::texture(int i) {
    return textures_[i];
}
