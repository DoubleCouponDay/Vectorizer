#pragma once

#include <vector>

#include "utility/pixel.h"

struct image
{
    image() : pixels() {}
    image(size_t width, size_t height) : pixels(width, std::vector<pixel>(height)) {}
    image(image&& other) noexcept : pixels(std::move(other.pixels)) {}

    image(const image& other) = delete;

    inline image& operator=(const image& other) = delete;
    inline image& operator=(image &&other) noexcept { pixels = std::move(other.pixels); other.pixels.clear(); return *this; }

    std::vector<std::vector<pixel>> pixels;

    inline size_t width() const noexcept { return pixels.size(); }
    inline size_t height() const noexcept { return (pixels.empty() ? 0 : pixels.front().size()); }

    const pixel& get(int x, int y) const
    {
        return pixels[x][y];
    }

    pixel& get(int x, int y)
    {
        return pixels[x][y];
    }

    inline void set(int x, int y, pixel pix) { pixels[x][y] = pix; }

    void clear(pixel color = pixel{ 0, 0, 0 });

    bool to_png(const char *file) const;
};
