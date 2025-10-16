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

template<int T> double dot(const vec<T>& a, const vec<T>& b) {
    double res = 0;
    for(int i=0; i<T; i++) res += a[i] * b[i];
    return res;
}

template<int T> std::ostream& operator<<(std::ostream& out, const vec<T>& v) {
    for(int i=0; i<T; i++) out << v[i] << " ";
    return out;
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
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

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
};