#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <cstdint>
#include <memory>
#include <string>
#include <stdexcept>

class Image
{
private:
    int m_width;
    int m_height;
    int m_channels;
    std::unique_ptr<uint8_t> m_pixels;
    Image() : m_width(0), m_height(0), m_channels(0), m_pixels(nullptr)
    {
    }

public:
    int getWidth() const
    {
        return m_width;
    }
    int getHeight() const
    {
        return m_height;
    }
    int getChannels() const
    {
        return m_channels;
    }
    int getBytesPerPixels() const
    {
        return sizeof(uint8_t);
    }
    int getPitch() const
    {
        return getBytesPerPixels() * m_width;
    }

    template <typename Type = uint8_t>
    const Type* getPixels() const
    {
        return reinterpret_cast<const Type*>(m_pixels.get());
    }

public: //static function
    static std::shared_ptr<Image> readImageFromFile(const std::string& filename)
    {
        std::shared_ptr<Image> image{ new Image };
        uint8_t* pixels = stbi_load(filename.c_str(), &image->m_width, &image->m_height, &image->m_channels, 3);
        if (pixels)
        {
            image->m_pixels.reset(pixels);
        }
        if (!image->m_pixels)
        {
            throw std::runtime_error("Failed to load image file: " + filename);
        }
        return image;
    }

    static void saveFileAsBitmap(const std::string& filename, int w, int h, const void* data)
    {
        stbi_write_bmp(filename.c_str(), w, h, 3, data);
    }
};