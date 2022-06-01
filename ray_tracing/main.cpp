//==============================================================================================
// Originally written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication
// along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==============================================================================================

#include "rtweekend.h"

#include "box.h"
#include "bvh.h"
#include "camera.h"
#include "color.h"
#include "constant_medium.h"
#include "hittable_list.h"
#include "material.h"
#include "moving_sphere.h"
#include "sphere.h"
#include "texture.h"

#include "triangle.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include "mat.h"
#include <array>
#include "stdio.h"

// Global Materials
auto my_metal   = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
auto my_glass   = make_shared<dielectric>(1.5);
auto my_diffuse = make_shared<lambertian>(color(0.7, 0.3, 0.3));
std::vector<shared_ptr<material>> materials = {my_metal, my_glass, my_diffuse};

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth-1);
}

// Global Teapot 
vector<std::array<double, 4>> teapot_pos;
vector<std::array<double, 3>> teapot_norm;
int teapot_vertex_cnt;

void load_teapot()
{
    std::ifstream teapot_file;
    teapot_file.open("modify_teapot.txt");

    std::string line;
    while(std::getline(teapot_file, line))
    {
        std::array<double, 4> pos;
        std::array<double, 3> norm;
        sscanf(line.c_str(), "%lf %lf %lf %lf %lf %lf", &pos[0], &pos[1], &pos[2], &norm[0], &norm[1], &norm[2]);
        pos[3] = 1.0;
        teapot_pos.push_back(pos);
        teapot_norm.push_back(norm);
    }
    teapot_vertex_cnt = teapot_pos.size();
}

void add_teapot(hittable_list& objects, mat4 pos_mat, mat3 norm_mat, shared_ptr<material> m) {

	hittable_list teapot;
    // for glass
    hittable_list inner_teapot; 
    bool is_glass = (m == my_glass);

    for(int i = 0; i < teapot_vertex_cnt / 3; i++)
    {
        vec3 pos[3];
        vec3 norm[3];
        vec3 inner_pos[3];
        for(int j = 0; j < 3; j++)
        {
            std::array<double, 4> new_pos;
            mat4_mul(pos_mat, teapot_pos[i * 3 + j], new_pos);
            std::array<double, 3> new_norm;
            mat3_mul(norm_mat, teapot_norm[i * 3 + j], new_norm);

            pos[j] = point3(new_pos[0], new_pos[1], new_pos[2]);
            norm[j] = vec3(new_norm[0], new_norm[1], new_norm[2]);
            norm[j] = normalize(norm[j]);
            if(is_glass)
            {
                double inner_mat[4][4];
                for(int k = 0; k < 4; k++)
                    for(int l = 0; l < 4; l++)
                        inner_mat[k][l] = pos_mat[k][l];
                for(int k = 0; k < 4; k++)
                    inner_mat[k][k] *= 0.95;
                mat4_mul(inner_mat, teapot_pos[i * 3 + j], new_pos);
                inner_pos[j] = point3(new_pos[0], new_pos[1], new_pos[2]);
            }
        }
        vec3 u = pos[1] - pos[0];
        vec3 v = pos[2] - pos[0];
        vec3 face_norm = normalize(cross(u, v));
        vec3 avg_vertex_norm = (norm[0] + norm[1] + norm[2]) / 3;
        face_norm = (dot(face_norm, avg_vertex_norm) > 0.0f)? face_norm : -face_norm;
        shared_ptr<hittable> tri = make_shared<triangle>(pos[0], pos[1], pos[2],
                                                         norm[0], norm[1], norm[2],
                                                         face_norm, m);
        teapot.add(tri);

        if(is_glass)
        {
            shared_ptr<hittable> inner_tri = make_shared<triangle>(inner_pos[0], inner_pos[1], inner_pos[2],
                                                                   -norm[0], -norm[1], -norm[2],
                                                                   -face_norm, m);
            inner_teapot.add(inner_tri);
        }
    }

	objects.add(make_shared<bvh_node>(teapot, 0, 0));
    if(is_glass)
    {
	    objects.add(make_shared<bvh_node>(inner_teapot, 0, 0));
    }
}

// Custom Properties
int samples_per_pixel;

// Image
auto aspect_ratio = 1.0;
int image_width = 400;
int image_height = 400;
int max_depth = 50;

// World
hittable_list world;

// Camera
camera cam;
color background;


// Global(output file)
std::vector<std::vector<std::string>> pixels;

void *render_thread(void *argv)
{
    int *range = (int *)argv;
    int min_width = range[0];
    int min_height = range[1];
    int max_width = range[2];
    int max_height = range[3];
    for (int j = max_height-1; j >= min_height; --j) {
        for (int i = min_width; i < max_width; ++i) {
            color pixel_color(0,0,0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, max_depth);
            }
            write_color(i, j, pixel_color, samples_per_pixel);
        }
    }
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

    // Arguments
    if(argc < 3)
    {
        std::cerr << "usage: ./a.out samples_per_pixel n [pos_mat norm_mat material] * n\n";
        std::cerr << "materials: (0) metal; (1) glass; (2) diffuse material\n";
        return -1;
    }
    samples_per_pixel = atoi(argv[1]);
    int number_of_teapot = atoi(argv[2]);
    if(argc - 3 !=  3 * number_of_teapot)
    {
        std::cerr << "number of file and material does not match\n";
        return -1;
    }
    mat4 pos_matices[number_of_teapot];
    mat3 norm_matices[number_of_teapot];
    int teapot_materials[number_of_teapot];
    for(int i = 0; i < number_of_teapot; i++)
    {
        create_mat4(pos_matices[i], argv[3 + 3 * i]);
        create_mat3(norm_matices[i], argv[3 + 3 * i + 1]);
        teapot_materials[i] = atoi(argv[3 + 3 * i + 2]);
    }

    // World
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    // Teapot
    load_teapot();

    world.add(make_shared<yz_rect>(-100, 100, -300,    0, -100, green));
    world.add(make_shared<yz_rect>(-100, 100, -300,    0,  100,   red));
    world.add(make_shared<xz_rect>( -25,  25, -175, -125,   96, light));
    world.add(make_shared<xz_rect>(-100, 100, -300,    0,  100, white));
    world.add(make_shared<xz_rect>(-100, 100, -300,    0, -100, white));
    world.add(make_shared<xy_rect>(-100, 100, -100,  100, -200, white));

    /*
    world.add(make_shared<triangle>(point3(-30, -50, -100),
                                    point3( 30, -50, -100),
                                    point3(  0, -50, -170),
                                    vec3(0,1,0),
                                    vec3(0,1,0),
                                    vec3(0,1,0),
                                    vec3(0,1,0),
                                    light));
    */

    for(int i = 0; i < number_of_teapot; i++)
    {
        add_teapot(world, pos_matices[i], norm_matices[i], materials[teapot_materials[i]]);
    }

    // Camera
    point3 lookfrom = point3(0, 0, 200);
    point3 lookat = point3(0, 0, -400);
    auto vfov = 40.0;
    auto aperture = 0.0;
    background = color(0.0,0.0,0.0);
    const vec3 vup(0,1,0);
    const auto dist_to_focus = 10.0;
    cam = camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
    
    // Render
    pixels.resize(image_width);
    for(int i = 0; i < image_width; i++)
    {
        pixels[i].resize(image_height, "");
    }

    pthread_t render_workers[16];
    int width_interval = image_width / 4;
    int height_interval = image_height / 4;
    int range[16][4];
    for(int j = 3; j >= 0; j--)
    {
        for(int i = 0; i < 4; i++)
        {
            int idx = i + 4 * j;
            range[idx][0] = i * width_interval;
            range[idx][1] = j * height_interval;
            range[idx][2] = (i + 1) * width_interval;
            range[idx][3] = (j + 1) * height_interval;
            pthread_create(&render_workers[idx], NULL, render_thread, (void *)range[idx]);
        }
    }

    for(int i = 0; i < 16; i++)
        pthread_join(render_workers[i], NULL);

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (int j = image_height-1; j >= 0; --j) 
        for (int i = 0; i < image_width; ++i) 
            std::cout << pixels[i][j];

    std::cerr << "\nDone.\n";
}
