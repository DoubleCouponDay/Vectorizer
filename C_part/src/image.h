#pragma once

#include <vector>
#include <string>

#include "utility/pixel.h"
#include "utility/vec.h"

namespace vectorizer
{

    class IPixelAccess
    {
    public:

        virtual size_t get_width() const = 0;
        virtual size_t get_height() const = 0;

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
        explicit Image(std::string path);

        Image(const Image& other) = delete;

        inline Image& operator=(const Image& other) = delete;
        inline Image& operator=(Image&& other) noexcept { pixels = std::move(other.pixels); other.pixels.clear(); return *this; }


        inline size_t width() const noexcept { return pixels.size(); }
        inline size_t height() const noexcept { return (pixels.empty() ? 0 : pixels.front().size()); }

        const pixel& get(int x, int y) const;
        pixel& get(int x, int y);

        inline pixel get_pixel(int x, int y) const override { return get(x, y); }
        inline void set_pixel(int x, int y, pixel pix) override { set(x, y, pix); }

        virtual size_t get_width() const override;
        virtual size_t get_height() const override;

        void set(int x, int y, pixel pix);

        bool empty() const;
        void clear(pixel color = pixel{ 0, 0, 0 });

        bool to_png(const char* file) const;

    private:

        std::vector<std::vector<pixel>> pixels;

    };

    template<class T>
    class Bitmap
    {
    public:
        explicit Bitmap() : _items{}, _index{ 0, 0 } {}
        Bitmap(size_t width, size_t height) : _items(width* height), _index{ width, height } {}

        Bitmap(Bitmap<T>&& other) noexcept : _items(std::move(other._items)), _index(other._index) { other._items.clear(); other._index.width = other._index.height = 0; }
        explicit Bitmap(const Bitmap<T>& other) : _items(other._items), _index(other._index) {}

        inline Bitmap& operator=(const Bitmap& other) { _items.assign(other._items.begin(), other._items.end()); _index = other._index; }
        inline Bitmap& operator=(Bitmap&& other) { _items = std::move(other._items); _index = other._index;  other._items.clear(); other._index.width = other._index.height = 0; }

        inline size_t width() const { return _index.width; }
        inline size_t height() const { return _index.height; }

        const T& get(int x, int y) const { return _items[_index(x, y)]; }
        T& get(int x, int y) { return _items[_index(x, y)]; }

        void set(int x, int y, const T& item) { _items[_index(x, y)] = item; }

    private:

        std::vector<T> _items;
        dimensional_indexer _index;
    };
}
