#include "geometry.h"
#include <vector>
#include <cassert>

Matrix::Matrix(int r, int c) : 
    m(std::vector<std::vector<float> >(r, std::vector<float>(c, 0.f))), rows(r), cols(c) {}

Matrix Matrix::identity(int dimensions) {
    Matrix I(dimensions, dimensions);
    for(int i=0; i<dimensions; i++)
    {
        I[i][i] = 1.f;
    }
    return I;
}

std::vector<float>& Matrix::operator[](const int i) {
    assert(0<=i && i<rows);
    return m[i];
}

Matrix Matrix::operator*(const Matrix& a) {
    assert(cols==a.rows);
    Matrix result(rows, a.cols);
    for(int i=0; i<rows; i++) {
        for(int j=0; j<a.cols; j++) {
            for(int k=0; k<cols; k++) {
                result.m[i][j] += m[i][k] * a.m[k][j];
            }
        }
    }
    return result;
}

Matrix Matrix::transpose() {
    Matrix result(rows, cols);
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            result[i][j] = m[i][j];
        }
    }
    return result;
}

Matrix Matrix::inverse() {
    assert(rows==cols);

    // inverting via an augmented matrix
    Matrix result(rows, cols*2);
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            result[i][j] = m[i][j];
        }
    }
    for(int i=0; i<rows; i++) {
        result[i][i+cols] = 1;
    }
    // forward elimination
    for(int i=0; i<rows-1; i++) {
        // normalize all rows
        for(int j=result.cols-1; j>=0; j--) {
            result[i][j]/=result[i][i];
        }
        // elimination
        for(int k=i+1; k<rows; k++) {
            float coeff = result[k][i];
            for(int j=0; j<result.cols; j++) {
                result[k][j] -= result[i][j]*coeff;
            }
        }
    }
    // normalize augmented matrix section
    for(int i=result.cols-1; i>=rows-1; i--) {
        result[rows-1][i] = result[rows-1][rows-1];
    }
    // backward elimination
    for(int i=rows-1; i>0; i--) {
        for(int j=i-1; j>=0; j--) {
            float coeff = result[j][i];
            for(int k=0; k<result.cols; k++) {
                result[j][k] -= result[i][j]*coeff;
            }
        }
    }
    // truncate to retrieve just the augmented section
    Matrix truncate(rows, cols);
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            truncate[i][j] = result[i][j + cols];
        }
    }
    return truncate;
}

std::ostream& operator<<(std::ostream& os, Matrix& m) {
    for(int i=0; i<m.nrows(); i++) {
        for(int j=0; j<m.ncols(); j++) {
            os << m[i][j] << "\t";
        }
        os << '\n';
    }
    return os;
}