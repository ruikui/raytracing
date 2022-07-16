#pragma once

#include "Vec3.h"
#include <algorithm>

using color = vec3;

void write_color(std::ostream& out, color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Divide the color by the number of samples and gamma-correct for gamma=2.0.
    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(256 * std::clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * std::clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * std::clamp(b, 0.0, 0.999)) << '\n';
}

void write_color(uint8_t* ptr, color pixel_color, int samples_per_pixel)
{
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Divide the color by the number of samples and gamma-correct for gamma=2.0.
    auto scale = 1.0 / samples_per_pixel;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    ptr[0] = static_cast<uint8_t>(256 * std::clamp(r, 0.0, 0.999));
    ptr[1] = static_cast<uint8_t>(256 * std::clamp(g, 0.0, 0.999));
    ptr[2] = static_cast<uint8_t>(256 * std::clamp(b, 0.0, 0.999));
}
