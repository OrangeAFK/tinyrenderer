#include <iostream>
#include <sstream>
#include "model.h"

Model::Model(const std::string filename) {
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
            vec3 v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts.push_back(v);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            vec3 n;
            for(int i=0; i<3; i++) iss >> n[i];
            norms.push_back(n.normalized());
        } else if (!line.compare(0, 2, "f ")) {
            int iF, iT, iN;
            iss >> trash;
            int cnt = 0;
            while (iss >> iF >> trash >> iT >> trash >> iN) {
                // in wavefront obj all indices start at 1, not zero
                facet_vert.push_back(--iF);
                facet_tex.push_back(--iT);
                facet_norm.push_back(--iF);
                cnt++;
            }
            if(cnt!=3) { std::cerr << "Error: obj file is not triangulated" << std::endl; return; }
        } else if(!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            vec2 uv;
            for (int i=0;i<2;i++) iss >> uv[i];
            tex_coord.push_back(uv);
        }
    }
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << " vt# " << tex_coord.size() << " vn# " << norms.size() << std::endl;
    load_texture(filename, "_diffuse.tga",    diffusemap );
    load_texture(filename, "_nm_tangent.tga", normalmap  );
    load_texture(filename, "_spec.tga",       specularmap);
}

int Model::nverts() const {
    return verts.size();
}
int Model::nfaces() const {
    return facet_vert.size()/3;
}
vec3 Model::normal(const vec2& uvf) const {
    TGAColor c = normalmap.get(uvf[0]*normalmap.width(), uvf[1]*normalmap.height());
    vec3 res;
    for (int i=0; i<3; i++)
        res[2-i] = (double)c[i]/255.f*2.f - 1.f;
    return res;
}
vec3 Model::normal(const int iface, const int nthvert) const {
    return norms[facet_vert[iface*3 + nthvert]];
}
vec3 Model::vert(const int i) const {
    return verts[i];
}
vec3 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vert[iface*3 + nthvert]];
}
vec2 Model::uv(const int iface, const int nthvert) const {
    return tex_coord[facet_tex[iface*3 + nthvert]];
}

const TGAColor Model::diffuse(const vec2& uvf) const {
    return diffusemap.get(uvf[0]*diffusemap.width(), uvf[1]*diffusemap.height());
}

void Model::load_texture(const std::string filename, const std::string suffix, TGAImage& image) {
    size_t dot = filename.find_last_of('.');
    if(dot==std::string::npos) return;
    std::string texfile = filename.substr(0,dot) + suffix;
    std::cerr << "texture file " << texfile << " loading " << (image.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
    image.flip_vertically();
}