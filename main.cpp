#include <cmath>
#include <iostream>
#include <string>
#include <algorithm>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <corecrt_math_defines.h>
using namespace std;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

mat<4> Viewport, Perspective, ModelView;
void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye-center);
    vec3 l = normalized(cross(up,n));
    vec3 m = normalized(cross(n, l));
    ModelView = mat<4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

void rasterize(const vec4 clip[3],  std::vector<double> &zbuffer, TGAImage &framebuffer, const TGAColor color){
    vec4 ndc[3]    = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w }; // normalized device coordinates [-1,1]x[-1,1]x[-1,1]
    vec2 screen[3] = { 
        (Viewport*ndc[0]).xy(),
        (Viewport*ndc[1]).xy(), 
        (Viewport*ndc[2]).xy()
    }; // screen coordinates [0,width]x[0,height]

    mat<3> ABC = {{ {screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.} }};
    if (ABC.det()<1) return; // backface culling + discarding triangles that cover less than a pixel

    auto [bbminx,bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // bounding box for the triangle
    auto [bbminy,bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // defined by its top left and bottom right corners
#pragma omp parallel for
    for (int x=std::max<int>(bbminx, 0); x<=std::min<int>(bbmaxx, framebuffer.width()-1); x++) { // clip the bounding box by the screen
        for (int y=std::max<int>(bbminy, 0); y<=std::min<int>(bbmaxy, framebuffer.height()-1); y++) {
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.}; // barycentric coordinates of {x,y} w.r.t the triangle
            if (bc.x<0 || bc.y<0 || bc.z<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            double z = dot(bc, vec3{ ndc[0].z, ndc[1].z, ndc[2].z });
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;
            zbuffer[x+y*framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    
    constexpr int width  = 800;
    constexpr int height = 800;

    constexpr vec3    eye{-1,0,2}; // camera position
    constexpr vec3 center{0,0,0};  // camera direction
    constexpr vec3     up{0,1,0};  // camera up vector

    lookat(eye, center, up); // build the ModelView   matrix
    vec3 f = eye - center;
    perspective(std::sqrt(dot(f,f))); // build the Perspective matrix
    viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport matrix

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height, -std::numeric_limits<double>::max());

    Model model(ASSET_DIR "/diablo3_pose.obj");
    for (int i = 0; i < model.nfaces(); i++) {
        vec4 clip[3];

        vec3 v0 = model.vert(i, 0);
        clip[0] = Perspective * ModelView * vec4{v0.x, v0.y, v0.z, 1.};
        vec3 v1 = model.vert(i, 1);
        clip[1] = Perspective * ModelView * vec4{v1.x, v1.y, v1.z, 1.};
        vec3 v2 = model.vert(i, 2);
        clip[2] = Perspective * ModelView * vec4{v2.x, v2.y, v2.z, 1.};

        rasterize(clip, zbuffer, framebuffer, white);
    }


    framebuffer.write_tga_file("framebuffer.tga");
    
    return 0;
}

