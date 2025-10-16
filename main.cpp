#include <cmath>
#include <iostream>
#include <string>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <corecrt_math_defines.h>
using namespace std;

constexpr int width  = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

vec3 rot(vec3 v) {
    constexpr double a = M_PI/6;
    mat<3> Ry = {{{std::cos(a), 0, std::sin(a)}, {0,1,0}, {-std::sin(a), 0, std::cos(a)}}};
    return Ry*v;
}

vec3 persp(vec3 v) {
    constexpr double c = 3.;
    return v / (1-v.z/c);
}

//Pass framebuffer by reference to avoid copying it
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    float y = ay;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        y += (by-ay) / static_cast<float>(bx-ax);
    }

}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
    
}

void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &framebuffer, float (&zbuffer_data)[width][height], TGAColor color) {

    int minX = min({ax, bx, cx});
    int maxX = max({ax, bx, cx});
    int minY = min({ay, by, cy});
    int maxY = max({ay, by, cy});
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return;

    #pragma omp parallel for
    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue;

            float z = alpha*az + beta*bz + gamma*cz;
            //if (z <= zbuffer.get(x, y)[0]) continue;
            //zbuffer.set(x, y, {z});
            if (z <= zbuffer_data[x][y]) continue;
            zbuffer_data[x][y] = z;
            framebuffer.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    float zbuffer_data [width][height]= {0.};

    Model model(ASSET_DIR "/diablo3_pose.obj");
    for (int i = 0; i < model.nfaces(); i++) {
        vec3 v0 = persp(rot(model.vert(i, 0)));
        vec3 v1 = persp(rot(model.vert(i, 1)));
        vec3 v2 = persp(rot(model.vert(i, 2)));
        int x0 = static_cast<int>((v0.x + 1.) * width / 2.);
        int y0 = static_cast<int>((v0.y + 1.) * height / 2.);
        int z0 = static_cast<int>((v0.z + 1.) * 128.);
        int x1 = static_cast<int>((v1.x + 1.) * width / 2.);
        int y1 = static_cast<int>((v1.y + 1.) * height / 2.);
        int z1 = static_cast<int>((v1.z + 1.) * 128.);
        int x2 = static_cast<int>((v2.x + 1.) * width / 2.);
        int y2 = static_cast<int>((v2.y + 1.) * height / 2.);
        int z2 = static_cast<int>((v2.z + 1.) * 128.);
        triangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, framebuffer, zbuffer_data, red);
    }

    for(int i=0; i<width; i++) {
        for(int j=0; j<height; j++) {

            uint8_t g = (uint8_t)std::lround(std::clamp((zbuffer_data[i][j]+1.f)*127.5f, 0.f, 255.f));
            zbuffer.set(i, j, {g});
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");
    
    return 0;
}

