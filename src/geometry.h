#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>
#include <cassert>
#include <iostream>

template <int n>
struct vec {
    double data[n] = { 0 };
    double& operator[](const int i) { assert(0<=i && i<n); return data[i]; }
    double operator[](const int i) const { assert(0<=i && i<n); return data[i]; }
    double norm() const { return std::sqrt(norm2()); }
    double norm2() const { return (*this) * (*this); }
    vec<n> normalized() const { return (*this)/norm(); }
};

template <int n>
vec<n> operator+(const vec<n>& lhs, const vec<n>& rhs) {
    vec<n> ret = lhs;
    for(int i=n; i--; ret[i]+=rhs[i]);
    return ret;
}

template <int n>
vec<n> operator-(const vec<n>& lhs, const vec<n>& rhs) {
    vec<n> ret = lhs;
    for(int i=n; i--; ret[i]-=rhs[i]);
    return ret;
}

template <int n>
double operator*(const vec<n>& lhs, const vec<n>& rhs) {
    // dot product
    double ret = 0;
    for(int i=n; i--; ret+= lhs[i]*rhs[i]);
    return ret;
}

template <int n>
vec<n> operator*(const double& lhs, const vec<n>& rhs) {
    vec<n> ret = rhs;
    for(int i=n; i--; ret[i]*=lhs );
    return ret;
}

template <int n>
vec<n> operator*(const vec<n>& lhs, const double& rhs) {
    return rhs * lhs;
}

template <int n>
vec<n> operator/(const vec<n>& lhs, const double& rhs) {
    assert(rhs != 0);
    vec<n> ret = lhs;
    for(int i=n; i--; ret[i]/=rhs);
    return ret;
}

template <int n1, int n2>
vec<n1> embed(const vec<n2>& v, double fill=1) {
    vec<n1> ret;
    for(int i=n1; i--; ret[i] = (i<n2 ? v[i] : fill));
    return ret;
}

template <int n1, int n2>
vec<n1> proj(const vec<n2>& v) {
    vec<n1> ret;
    for(int i=n1; i--; ret[i]=v[i]);
    return ret;
}

template <int n>
std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
    for(int i=0; i<n; i++) out<<v[i]<<' ';
    return out;
}

template <>
struct vec<2>
{
    double x=0, y=0;
    double& operator[](const int i) { assert(0<=i && i<2); return (i ? y : x); }
    double operator[](const int i) const { assert(0<=i && i<2); return (i ? y : x); }
    double norm() const { return std::sqrt(norm2()); }
    double norm2() const { return (*this) * (*this); }
    vec<2> normalized() const { return (*this)/norm(); }
};

template <>
struct vec<3>
{
    double x=0, y=0, z=0;
    double& operator[](const int i) { assert(0<=i && i<3); return (i ? (i==1 ? y : z) : x); }
    double operator[](const int i) const { assert(0<=i && i<3); return (i ? (i==1 ? y : z) : x); }
    double norm() const { return std::sqrt(norm2()); }
    double norm2() const { return (*this) * (*this); }
    vec<3> normalized() const { return (*this)/norm(); }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

inline vec3 cross(const vec3& v1, const vec3& v2) {
    return vec3{v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x};
}

template<int n> struct dt;

template<int r, int c> struct mat {
    // initialize to zero array
    vec<c> rows[r] = { {} };

    // setters/getters for rows and cols (operator[] for rows, methods for cols)
    vec<c>& operator[](const int i) { assert(0<=i && i<r); return rows[i]; }
    const vec<c>& operator[](const int i) const { assert(0<=i && i<r); return rows[i]; }

    vec<r> col(const int i) const {
        assert(0<=i && i<c);
        vec<r> ret;
        for(int j=r; j--; ret[j] = rows[j][i]);
        return ret;
    }

    void set_col(const int i, vec<r>& v) {
        assert(0<=i && i<c);
        for(int j=r; j--; rows[j][i] = v[j] );
    }

    // return identity matrix
    static mat<r, c> identity() {
        assert(r==c);
        mat<r,c> ret;
        for(int i=r; i--; ) for(int j=c; j--; ret[i][j]=(i==j));
        return ret;
    }
    
    // determinant
    double det() const {
        return dt<c>::det(*this);
    }

    // get_minor
    mat<r-1, c-1> get_minor(const int row, const int col) const {
        mat<r-1, c-1> ret;
        for (int i = 0; i < r - 1; i++)
            for (int j = 0; j < c - 1; j++)
                ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1];
        return ret;
    }

    // get cofactor
    double cofactor(const int i, const int j) const {
        return get_minor(i, j).det() * ((i + j) % 2 ? -1 : 1);
    } 

    // get adjugate matrix
    mat<r, c> adjugate() const {
        mat<r, c> ret;
        for(int i=r; i--; ) for(int j=c; j--; ret[i][j] = cofactor(i, j));
        return ret;
    }

    // get inverse transpose
    mat<r, c> inverse_transpose() const {
        mat<r, c> adj = adjugate();
        return adj / (adj[0] * rows[0]);
    }

    // get inverse
    mat<r, c> inverse() const {
        return inverse_transpose().transpose();
    }

    // get transpose
    mat<c, r> transpose() const {
        mat<c, r> ret;
        for(int i=c; i--; ret[i] = this->col(i));
        return ret;
    }
};

template<int r, int c>
mat<r, c> operator+(const mat<r,c>& lhs, const mat<r,c>& rhs) {
    mat<r, c> ret;
    for(int i=r; i--; ) for(int j=c; j--; ret[i][j] = lhs[i][j] + rhs[i][j]);
    return ret;
}

template<int r, int c>
mat<r, c> operator-(const mat<r,c>& lhs, const mat<r,c>& rhs) {
    mat<r, c> ret;
    for(int i=r; i--; ) for(int j=c; j--; ret[i][j] = lhs[i][j] - rhs[i][j]);
    return ret;
}

// matrix-vector, matrix-matrix, and scalar-matrix multiplications
template<int r, int c>
vec<r> operator*(const mat<r,c>& lhs, const vec<c>& rhs) {
    vec<r> ret;
    for(int i=r; i--; ret[i] = lhs[i] * rhs);
    return ret;
}

template<int l, int m, int n>
mat<l, n> operator*(const mat<l, m>& lhs, const mat<m, n>& rhs) {
    mat<l, n> ret;
    for(int i=l; i--; ) for(int j=n; j--; ret[i][j] = lhs[i]*rhs.col(j));
    return ret;
}

template<int r, int c>
mat<r, c> operator*(const mat<r, c>& lhs, const double val) {
    mat<r, c> ret;
    for(int i=r; i--; ret[i] = lhs[i]*val );
    return ret;
}

template<int r, int c>
mat<r, c> operator*(const double val, const mat<r, c>& rhs) {
    return rhs*val;
}

template<int r, int c>
mat<r, c> operator/(const mat<r, c>& lhs, const double val) {
    assert(val != 0);
    return lhs * (1/val);
}

template<int r, int c>
std::ostream& operator<<(std::ostream& os, const mat<r, c>& m) {
    for(int i=0; i<r; i++) os<<m[i]<<std::endl;
    return os;
}

template<int n> struct dt {
    static double det(const mat<n,n>& src) {
        double ret = 0;
        for (int i=n; i--; ret += src[0][i]*src.cofactor(0,i));
        return ret;
    }
};

template<> struct dt<1> {
    static double det(const mat<1,1>& src) {
        return src[0][0];
    }
};

#endif