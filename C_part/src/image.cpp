#include "image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "utility/error.h"
#include "utility/vec.h"

#include "imagefile/pngfile.h"

namespace vectorizer
{
    const pixel& Image::get(int x, int y) const
    {
        return pixels[x][y];
    }

    pixel& Image::get(int x, int y)
    {
        return pixels[x][y];
    }

    int Image::get_width() const
    {
        return width();
    }

    int Image::get_height() const
    {
        return height();
    }

    void Image::set(int x, int y, pixel pix)
    {
        pixels[x][y] = pix;
    }

    bool Image::empty() const
    {
        return width() < 1 || height() < 1;
    }

    bool Image::to_png(const char* file) const
    {
        write_image_to_png(*this, file);

        return getAndResetErrorCode() != SUCCESS_CODE;
    }

    void Image::clear(pixel color)
    {
        for (int x = 0; x < width(); ++x)
        {
            for (int y = 0; y < height(); ++y)
            {
                set(x, y, color);
            }
        }
    }


}