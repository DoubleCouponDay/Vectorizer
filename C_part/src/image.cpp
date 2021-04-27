#include "image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "utility/error.h"
#include "utility/vec.h"

#include "imagefile/pngfile.h"

bool image::to_png(const char *file) const
{
    write_image_to_png(*this, file);

    return getAndResetErrorCode() != SUCCESS_CODE;
}

void image::clear(pixel color)
{
    for (int x = 0; x < width(); ++x)
    {
        for (int y = 0; y < height(); ++y)
        {
            set(x, y, color);
        }
    }
}