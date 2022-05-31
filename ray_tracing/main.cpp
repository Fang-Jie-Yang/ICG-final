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

void add_teapot(hittable_list& objects, std::ifstream &file, shared_ptr<material> m) {
    std::string vertex_pos_str;
    std::string vertex_norm_str;
    std::getline(file, vertex_pos_str);
    std::getline(file, vertex_norm_str);
    std::istringstream vertex_pos_ss(vertex_pos_str);
    std::istringstream vertex_norm_ss(vertex_norm_str);

    bool is_glass = (m == my_glass);
    if(is_glass)
    {
        std::cerr << "hi, glass\n";
    }

	hittable_list teapot;
    hittable_list inner_teapot; // for glass
    std::string val;
    int val_cnt = 0;
    double vals[3];
    int point_cnt = 0;
    point3 points[3];
    while(getline(vertex_pos_ss, val, ','))
    {
        vals[val_cnt] = stod(val);
        val_cnt++;
        if(val_cnt == 3)
        {
            // to be fixed
            points[point_cnt] = point3(vals[0] * 10, vals[1] * 10, vals[2] * 10);
            point_cnt++;
            val_cnt = 0;
        }
        if(point_cnt == 3)
        {
            double avg_vertex_norm[3] = {0, 0, 0};
            for(int i = 0; i < 9; i++)
            {
                getline(vertex_norm_ss, val, ',');
                avg_vertex_norm[i % 3] += stod(val) / 3;
            }
            vec3 vertex_norm(avg_vertex_norm[0], avg_vertex_norm[1], avg_vertex_norm[2]);
            vec3 u = 0.5 * (points[1] - points[0]);
            vec3 v = 0.5 * (points[2] - points[0]);
            vec3 face_norm = cross(u, v);
            double d = dot(vertex_norm, face_norm);
            face_norm = (d > 0.0f)? face_norm : -face_norm;
            shared_ptr<hittable> tri = make_shared<triangle>(points[0], points[1], points[2], face_norm, m);
            tri = make_shared<translate>(tri, vec3(0,-150,0));
            teapot.add(tri);
            if(is_glass)
            {
                shared_ptr<hittable> inner_tri = make_shared<triangle>(0.95*points[0], 0.95*points[1], 0.95*points[2], -face_norm, m);
                inner_tri = make_shared<translate>(tri, vec3(0,-150,0));
                inner_teapot.add(inner_tri);
            }
            point_cnt = 0;
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
        //std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
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
        std::cerr << "usage: ./a.out samples_per_pixel number_of_teapot [teapot file names] [teapots' materials]\n";
        std::cerr << "materials: (0) metal; (1) glass; (2) diffuse material\n";
        return -1;
    }
    samples_per_pixel = atoi(argv[1]);
    int number_of_teapot = atoi(argv[2]);
    if(argc - 3 !=  2 * number_of_teapot)
    {
        std::cerr << "number of file and material does not match\n";
        return -1;
    }
    std::vector<std::ifstream> teapot_files(number_of_teapot);
    std::vector<int> teapot_materials(number_of_teapot);
    for(int i = 0; i < number_of_teapot; i++)
    {
        teapot_files[i].open(argv[3 + i]);
        teapot_materials[i] = atoi(argv[3 + number_of_teapot + i]);
    }

    // World
    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    world.add(make_shared<yz_rect>(-278, 278, -300, 255,  278, green));
    world.add(make_shared<yz_rect>(-278, 278, -300, 255, -278,   red));
    world.add(make_shared<xz_rect>( -65,  65, -123,  82,  266, light));
    world.add(make_shared<xz_rect>(-278, 278, -300, 255,  278, white));
    world.add(make_shared<xz_rect>(-278, 278, -300, 255, -278, white));
    world.add(make_shared<xy_rect>(-278, 278, -278, 278,  255, white));
    for(int i = 0; i < number_of_teapot; i++)
    {
        add_teapot(world, teapot_files[i], materials[teapot_materials[i]]);
    }

    // Camera
    point3 lookfrom = point3(0, 0, -800);
    point3 lookat = point3(0, 0, 0);
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
    {
        pthread_join(render_workers[i], NULL);
    }
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (int j = image_height-1; j >= 0; --j) 
    {
        for (int i = 0; i < image_width; ++i) 
        {
            std::cout << pixels[i][j];
        }
    }

    std::cerr << "\nDone.\n";
}
