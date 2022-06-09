#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "rtweekend.h"

#include "hittable.h"


using std::max;
using std::min;

class triangle : public hittable
{
    public:
        triangle() {}
        triangle(point3 a, point3 b, point3 c, 
                 vec3 n_a, vec3 n_b, vec3 n_c, 
                 vec3 norm,
                 shared_ptr<material> m) 
            : a(a), b(b), c(c), n_a(n_a), n_b(n_b), n_c(n_c), norm(norm), mat_ptr(m) {};

        virtual bool hit(const ray &r, double t_min, double t_max, hit_record &rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

    public:
        point3 a, b, c;
        vec3 n_a, n_b, n_c;
        vec3 norm;
        shared_ptr<material> mat_ptr;
};

bool triangle::bounding_box(double time0, double time1, aabb& output_box) const
{
    double x_max = max(a.x(), max(b.x(), c.x()));
    double y_max = max(a.y(), max(b.y(), c.y()));
    double z_max = max(a.z(), max(b.z(), c.z()));
    double x_min = min(a.x(), min(b.x(), c.x()));
    double y_min = min(a.y(), min(b.y(), c.y()));
    double z_min = min(a.z(), min(b.z(), c.z()));
    output_box = aabb(
        vec3(x_min - 0.001, y_min - 0.001, z_min - 0.001),
        vec3(x_max + 0.001, y_max + 0.001, z_max + 0.001));
    return true;
}


bool triangle::hit(const ray &r, double t_min, double t_max, hit_record &rec) const
{
    // Moller-Trumbore
    vec3 E1 = b - a;
    vec3 E2 = c - a;
    vec3 P = cross(r.direction(), E2);
    double det = dot(P, E1);

    if(det < 0.0001)
        return false;

    double inv = 1 / det;
    vec3 T = r.origin() - a;
    double u = inv * dot(P, T);
    if(u < 0 || u > 1)
        return false;

    vec3 Q = cross(T, E1);
    double v = inv * dot(Q, r.direction());
    if(v < 0 || u + v > 1)
        return false;

    double t = inv * dot(Q, E2);

    if(t < t_min || t > t_max)
        return false;

    rec.p = r.origin() + t * r.direction();
    rec.t = t;
    rec.mat_ptr = mat_ptr;

    vec3 normal = normalize((1 - u - v) * n_a + u * n_b + v * n_c);
    rec.set_face_normal(r, normal);

    return true;

/*  Algebraic Method

    // intersect ray with plane
    vec3   u = b - a;
    vec3   v = c - a;
    double d = - dot(a, norm);
    double t = - (dot(r.origin(), norm) + d) / dot(r.direction(), norm);
    point3 P = r.origin() + t * r.direction();

    if(t < t_min || t > t_max)
        return false;

    // check if point P is inside the triangle
    vec3 V_1;
    vec3 V_2;
    vec3 N_1;
    V_1 = a - P;
    V_2 = b - P;
    N_1 = cross(V_2, V_1);
    if(dot(r.direction(), N_1) < 0)
        return false;
    V_1 = b - P;
    V_2 = c - P;
    N_1 = cross(V_2, V_1);
    if(dot(r.direction(), N_1) < 0)
        return false;
    V_1 = c - P;
    V_2 = a - P;
    N_1 = cross(V_2, V_1);
    if(dot(r.direction(), N_1) < 0)
        return false;

    rec.p = P;
    rec.t = t;
    rec.mat_ptr = mat_ptr;
    rec.set_face_normal(r, norm);

    return true;
*/
}

#endif
