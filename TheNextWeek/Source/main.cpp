#include "Common.h"
#include "Color.h"
#include "HittableList.h"
#include "Sphere.h"
#include "Camera.h"
#include "Vec3.h"
#include "Ray.h"
#include "Material.h"
#include "Image.h"
#include "MovingSphere.h"
#include "Threadpool.h"
#include "Texture.h"
#include "AARect.h"
#include "Box.h"
#include "ConstantMedium.h"
#include "BVHNode.h"
using namespace std;

#define WITH_PPM 0
#define WITH_BMP 1

color ray_color(const ray& r, const color&background, const hittable& world, int depth) {
    hit_record rec;
    if (depth <= 0)
    {
        return color(0, 0, 0);
    }

    if (!world.hit(r, 0.001, infinity, rec))
    {
        return background;
    }
    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
    {
        return emitted;
    }
    return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

hittable_list random_scene()
{
    hittable_list world;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9)
            {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8)
                {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0, .5), 0);
                    world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else
                {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }
    
    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

hittable_list two_spheres()
{
    hittable_list objects;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list two_perlin_spheres() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

hittable_list earth() {
    auto earth_texture = make_shared<image_texture>(TEXTURE_DIR + "/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

hittable_list simple_light()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list cornell_box()
{
    hittable_list objects;

    auto red = std::make_shared<lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<lambertian>(color(.12, .45, .15));
    auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(std::make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(std::make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(std::make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(std::make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(std::make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(std::make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    std::shared_ptr<hittable> box1 = std::make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265, 0, 295));
    objects.add(box1);

    std::shared_ptr<hittable> box2 = std::make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = std::make_shared<rotate_y>(box2, -18);
    box2 = std::make_shared<translate>(box2, vec3(130, 0, 65));
    objects.add(box2);

    return objects;
}

hittable_list cornell_smoke() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    std::shared_ptr<hittable> box1 = std::make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265, 0, 295));

    std::shared_ptr<hittable> box2 = std::make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = std::make_shared<rotate_y>(box2, -18);
    box2 = std::make_shared<translate>(box2, vec3(130, 0, 65));

    objects.add(std::make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
    objects.add(std::make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

    return objects;
}

hittable_list final_scene()
{
    hittable_list boxes1;
    auto ground = std::make_shared<lambertian>(color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++)
    {
        for (int j = 0; j < boxes_per_side; j++)
        {
            auto w = 100.0;
            auto x0 = -1000.0 + i * w;
            auto z0 = -1000.0 + j * w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            boxes1.add(std::make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
        }
    }

    hittable_list objects;

    objects.add(std::make_shared<bvh_node>(boxes1, 0, 1));

    auto light = std::make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(std::make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30, 0, 0);
    auto moving_sphere_material = std::make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(std::make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

    objects.add(std::make_shared<sphere>(point3(260, 150, 45), 50, std::make_shared<dielectric>(1.5)));
    objects.add(std::make_shared<sphere>(point3(0, 150, 145), 50, std::make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));

    auto boundary = std::make_shared<sphere>(point3(360, 150, 145), 70, std::make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(std::make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = std::make_shared<sphere>(point3(0, 0, 0), 5000, std::make_shared<dielectric>(1.5));
    objects.add(std::make_shared<constant_medium>(boundary, 0.0001, color(1, 1, 1)));

    auto emat = std::make_shared<lambertian>(std::make_shared<image_texture>(TEXTURE_DIR + "/earthmap.jpg"));
    objects.add(std::make_shared<sphere>(point3(400, 200, 400), 100, emat));
    auto pertext = std::make_shared<noise_texture>(0.1);
    objects.add(std::make_shared<sphere>(point3(220, 280, 300), 80, std::make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(std::make_shared<sphere>(point3::random(0, 165), 10, white));
    }

    objects.add(std::make_shared<translate>(
        std::make_shared<rotate_y>(
            std::make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
        vec3(-100, 270, 395)
        )
    );

    return objects;
}

int main() {

    // Image
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int samples_per_pixel = 100;
    const int max_depth = 50;

    // World
    hittable_list world;
    color background(0, 0, 0);
    ThreadPool threadPool(std::thread::hardware_concurrency() - 1);

    // Camera
    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;

    switch (2) {
    case 1:
        world = random_scene();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        break;

    case 2:
        world = two_spheres();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;
        
    case 3:
        world = two_perlin_spheres();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;

    case 4:
        world = earth();
        background = color(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;

    case 5:
        world = simple_light();
        samples_per_pixel = 400;
        background = color(0, 0, 0);
        lookfrom = point3(26, 3, 6);
        lookat = point3(0, 2, 0);
        vfov = 20.0;
        break;

    case 6:
        world = cornell_box();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 200;
        background = color(0, 0, 0);
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;

    case 7:
        world = cornell_smoke();
        aspect_ratio = 1.0;
        image_width = 600;
        samples_per_pixel = 200;
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;

    default:
    case 8:
        world = final_scene();
        aspect_ratio = 1.0;
        image_width = 800;
        samples_per_pixel = 10000;
        background = color(0, 0, 0);
        lookfrom = point3(478, 278, -600);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;
    }

    // Camera

    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    int image_height = static_cast<int>(image_width / aspect_ratio);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Render
#if WITH_PPM
    std::ofstream outfile(IAMGE_OUTPUT_DIR + "/output.ppm");
    outfile << "P3\n" << image_width << ' ' << image_height << "\n255\n";
#elif WITH_BMP
    std::string image_path = IAMGE_OUTPUT_DIR + "/mt.bmp";
    uint8_t* data = new uint8_t[image_width * image_height * 3];
    uint8_t* ptr = data;
#endif

    for (int j = image_height - 1; j >= 0; --j)
    {
        //std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;

        auto render_func = [&](int line, int width, uint8_t* p)
        {
            for (int i = 0; i < width; ++i)
            {
                color pixel_color(0, 0, 0);
                for (int s = 0; s < samples_per_pixel; ++s)
                {
                    auto u = (i + random_double()) / (image_width - 1);
                    auto v = (line + random_double()) / (image_height - 1);
                    ray r = cam.get_ray(u, v);
                    pixel_color += ray_color(r, background, world, max_depth);
                }
                write_color(p, pixel_color, samples_per_pixel);
                p += 3;
            }
        };

        auto p = ptr;
        threadPool.submit(render_func, j, image_width, p);
        ptr += image_width * 3;

    }
#if WITH_BMP
    threadPool.wait();
    Image::saveFileAsBitmap(image_path, image_width, image_height, data);
#endif
    std::cerr << "\nDone.\n";
}