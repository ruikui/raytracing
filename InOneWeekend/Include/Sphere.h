#pragma once

#include "Hittable.h"

class sphere :public hittable
{
public:
    sphere() {}
    sphere(point3 c,double r,std::shared_ptr<material> m):center(c),radius(r),mat_ptr(m){}

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override
    {
        vec3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = half_b * half_b - a * c;
        if (discriminant < 0)
        {
            return false;
        }
        auto sqrtd = std::sqrt(discriminant);
        auto root = (-half_b - sqrtd) / a;
        if (root < t_min || t_max < root)
        {
            root = (-half_b + sqrtd) / a;
            if (root < t_min || t_max < root)
            {
                return false;
            }
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat_ptr = mat_ptr;

        return true;
    }

public:
    point3 center;
    double radius;
    std::shared_ptr<material> mat_ptr;
};