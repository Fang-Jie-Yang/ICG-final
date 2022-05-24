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

void add_teapot(hittable_list& objects, shared_ptr<material> m) {
    std::ifstream file;
    file.open("Teapot.txt");
    std::string vertex_pos_str;
    std::string vertex_norm_str;
    std::getline(file, vertex_pos_str);
    std::getline(file, vertex_norm_str);
    std::istringstream vertex_pos_ss(vertex_pos_str);
    std::istringstream vertex_norm_ss(vertex_norm_str);


	hittable_list teapot;
    std::string val;
    int val_cnt = 0;
    double vals[3];
    int point_cnt = 0;
    point3 points[3];
    while(getline(vertex_pos_ss, val, ','))
    {
        //std::cout << val << "\n";
        vals[val_cnt] = stod(val);
        val_cnt++;
        if(val_cnt == 3)
        {
            points[point_cnt] = point3(vals[0]*10, vals[1]*10, vals[2]*10);
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
            // to be fixed(tranformation of vertex norm)
            vec3 vertex_norm(avg_vertex_norm[0], avg_vertex_norm[1], avg_vertex_norm[2]);
            vec3 u = 0.5 * (points[1] - points[0]);
            vec3 v = 0.5 * (points[2] - points[0]);
            vec3 face_norm = cross(u, v);
            double d = dot(vertex_norm, face_norm);
            face_norm = (d > 0.0f)? face_norm : -face_norm;
            shared_ptr<hittable> tri = make_shared<triangle>(points[0], points[1], points[2], face_norm, m);
            tri = make_shared<translate>(tri, vec3(0,-150,0));
            teapot.add(tri);
            point_cnt = 0;
        }
    }
	objects.add(make_shared<bvh_node>(teapot, 0, 0));
}

hittable_list cornell_box() {
    hittable_list objects;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    /*
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
    */

    objects.add(make_shared<yz_rect>(0 - 278, 555 - 278, 0 - 300, 555 - 300, 555 - 278, green));
    objects.add(make_shared<yz_rect>(0 - 278, 555 - 278, 0 - 300, 555 - 300, 0 - 278, red));
    objects.add(make_shared<xz_rect>(213 - 278,
									 343 - 278,
									 227 - 50 - 300,
									 332 + 50 - 300,
									 554 - 278,
									 light));
    objects.add(make_shared<xz_rect>(0 - 278, 555 - 278, 0 - 300, 555 - 300, 555 - 278, white));
    objects.add(make_shared<xz_rect>(0 - 278, 555 - 278, 0 - 300, 555 - 300, 0 - 278, white));
    objects.add(make_shared<xy_rect>(0 - 278, 555 - 278, 0 - 278, 555 - 278, 555 - 300, white));

    /*
    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    objects.add(box2);
    */

    //shared_ptr<hittable> ball = make_shared<sphere>(point3(0, 0, -2), 2, light);
    //objects.add(ball);
    /*
    point3 a(555, 10, 300);
    point3 b(0, 10, 300);
    point3 c(227, 300, 500);
    vec3 u = b - a;
    vec3 v = c - a;
    vec3 n = cross(u, v);
    shared_ptr<hittable> tri = make_shared<triangle>(a, b, c, n, light);
    objects.add(tri);
    */
	auto silver = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);

    add_teapot(objects, silver);

    return objects;
}

int main() {

    // Image

    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int samples_per_pixel = 10;
    int max_depth = 50;

    // World

    hittable_list world;

    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0.0,0.0,0.0);

    world = cornell_box();
    aspect_ratio = 1.0;
    image_width = 400;
    samples_per_pixel = 1000;
    //lookfrom = point3(278, 278, -800);
    lookfrom = point3(0, 0, -800);
    //lookat = point3(278, 278, 0);
    lookat = point3(0, 0, 0);
    vfov = 40.0;
            
    // Camera

    const vec3 vup(0,1,0);
    const auto dist_to_focus = 10.0;
    const int image_height = static_cast<int>(image_width / aspect_ratio);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Render

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height-1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0,0,0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, max_depth);
            }
            write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }

    std::cerr << "\nDone.\n";
}
