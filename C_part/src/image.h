#pragma once

#include <vector>

#include "utility/pixel.h"
#include "utility/vec.h"

namespace vectorizer
{

    class IPixelAccess
    {
    public:

        virtual int get_width() const = 0;
        virtual int get_height() const = 0;

        virtual pixel get_pixel(int x, int y) const = 0;
        inline virtual pixel get_pixel(vector2i pos) const { return this->get_pixel(pos.x, pos.y); }

        virtual void set_pixel(int x, int y, pixel pix) = 0;
        inline virtual void set_pixel(vector2i pos, pixel pix) { this->set_pixel(pos.x, pos.y, pix); }
    };

    class Image : public IPixelAccess
    {
    public:
        Image() : pixels() {}
        Image(size_t width, size_t height) : pixels(width, std::vector<pixel>(height)) {}
        Image(Image&& other) noexcept : pixels(std::move(other.pixels)) {}

        Image(const Image& other) = delete;

        inline Image& operator=(const Image& other) = delete;
        inline Image& operator=(Image&& other) noexcept { pixels = std::move(other.pixels); other.pixels.clear(); return *this; }


        inline size_t width() const noexcept { return pixels.size(); }
        inline size_t height() const noexcept { return (pixels.empty() ? 0 : pixels.front().size()); }

        const pixel& get(int x, int y) const;
        pixel& get(int x, int y);

        inline pixel get_pixel(int x, int y) const override { return get(x, y); }
        inline void set_pixel(int x, int y, pixel pix) override { set(x, y, pix); }

        virtual int get_width() const override;
        virtual int get_height() const override;

        void set(int x, int y, pixel pix);

        bool empty() const;
        void clear(pixel color = pixel{ 0, 0, 0 });

        bool to_png(const char* file) const;

    private:

        std::vector<std::vector<pixel>> pixels;

    };
}
