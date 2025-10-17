#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<int T> struct vec{
    double v[T];
    double& operator[](const int i)       { assert(i>=0 && i<T); return v[i]; }
    double  operator[](const int i) const { assert(i>=0 && i<T); return v[i]; }
};

template<int T> vec<T> operator+(const vec<T>& a, const vec<T>& b) {
    vec<T> res;
    for(int i=0; i<T; i++) res[i] = a[i] + b[i];
    return res;
}

template<int T> vec<T> operator-(const vec<T>& a, const vec<T>& b) {
    vec<T> res;
    for(int i=0; i<T; i++) res[i] = a[i] - b[i];
    return res;
}

template<int T> vec<T> operator/(const vec<T>& a, const double& f) {
    vec<T> res;
    for(int i=0; i<T; i++) res[i] = a[i] / f;
    return res;
}

template<int T> vec<T> operator*(const double& f, const vec<T>& a) {
    vec<T> res;
    for(int i=0; i<T; i++) res[i] = a[i] * f;
    return res;
}

template<int T> double dot(const vec<T>& a, const vec<T>& b) {
    double res = 0;
    for(int i=0; i<T; i++) res += a[i] * b[i];
    return res;
}

template<int T> std::ostream& operator<<(std::ostream& out, const vec<T>& v) {
    for(int i=0; i<T; i++) out << v[i] << " ";
    return out;
}

template<int T> vec<T> normalized(const vec<T>& v) {
    return v / std::sqrt(dot(v,v));
}

template<> struct vec<2> {
    double x = 0 ,y = 0;
    double& operator[](const int i)       { assert(i>=0 && i<2); return i ? y : x; }
    double  operator[](const int i) const { assert(i>=0 && i<2); return i ? y : x; }

};

template<> struct vec<3> {
    double x = 0, y = 0, z = 0;
    double& operator[](const int i)       { assert(i>=0 && i<3); return i==0 ? x : (i==1 ? y : z); }
    double  operator[](const int i) const { assert(i>=0 && i<3); return i==0 ? x : (i==1 ? y : z); }
};

template<> struct vec<4> {
    double x = 0, y = 0, z = 0, w = 0;
    double& operator[](const int i)       { assert(i>=0 && i<4); return i==0 ? x : (i==1 ? y : (i==2 ? z : w)); }
    double  operator[](const int i) const { assert(i>=0 && i<4); return i==0 ? x : (i==1 ? y : (i==2 ? z : w)); }
    vec<2> xy() const { return vec<2>{x, y}; }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

inline vec3 cross(const vec<3>& a, const vec<3>& b) {
    vec<3> res;
    res[0] = a[1]*b[2] - a[2]*b[1];
    res[1] = a[2]*b[0] - a[0]*b[2];
    res[2] = a[0]*b[1] - a[1]*b[0];
    return res;
}

template<int T> struct mat{
    vec<T> rows[T];
    vec<T>& operator[](const int i)       { assert(i>=0 && i<T); return rows[i]; }
    vec<T>  operator[](const int i) const { assert(i>=0 && i<T); return rows[i]; }
    static mat identity() {
        mat<T> I;
        for(int i=0; i<T; i++) 
            for(int j=0; j<T; j++) 
                I[i][j] = (i==j);
        return I;
    }
    mat operator*(const mat& m) {
        mat<T> res;
        for(int i=0; i<T; i++)
            for(int j=0; j<T; j++) {
                res[i][j] = 0;
                for(int k=0; k<T; k++)
                    res[i][j] += rows[i][k] * m[k][j];
            }
        return res;
    }
    vec<T> operator*(const vec<T>& v) {
        vec<T> res;
        for(int i=0; i<T; i++) {
            res[i] = 0;
            for(int j=0; j<T; j++)
                res[i] += rows[i][j] * v[j];
        }
        return res;
    }

    double det() const {
        assert(T>=1);
        if constexpr (T==1) {
            return rows[0][0];
        } else if constexpr (T==2) {
            return rows[0][0]*rows[1][1] - rows[0][1]*rows[1][0];
        } else {
            double determinant = 0;
            for (int c = 0; c < T; c++) {
                mat<T-1> submatrix;
                for (int i = 1; i < T; i++) {
                    int sub_j = 0;
                    for (int j = 0; j < T; j++) {
                        if (j == c) continue;
                        submatrix[i-1][sub_j] = rows[i][j];
                        sub_j++;
                    }
                }
                determinant += (c % 2 == 0 ? 1 : -1) * rows[0][c] * submatrix.det();
            }
            return determinant;
        }
    }

    double cofactor(const int row, const int col) const {
        mat<T-1> submatrix;
        for (int i=T-1; i--; )
            for (int j=T-1;j--; submatrix[i][j]=rows[i+int(i>=row)][j+int(j>=col)]);
        return submatrix.det() * ((row+col)%2 ? -1 : 1);
    }

    mat<T> invert_transpose() const {
        mat<T> adjugate_transpose; // transpose to ease determinant computation, check the last line
        for (int i=T; i--; )
            for (int j=T; j--; adjugate_transpose[i][j]=cofactor(i,j));
        return adjugate_transpose/(dot(adjugate_transpose[0],rows[0]));
    }

    
};

template<int T>mat<T> operator/(const mat<T>& lhs, const double& val) {
    mat<T> result;
    for (int i=T; i--; result[i] = lhs[i]/val);
    return result;
}