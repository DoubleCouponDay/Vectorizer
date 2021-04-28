#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <nanosvg.h>

#include "image.h"
#include "chunkmap.h"
#include "utility/vec.h"

namespace vectorizer
{
    enum mapping_consts {
        BEZIERCURVE_LENGTH = 8,
        BOUNDS_LENGTH = 4,
        ID_LENGTH = 64,
        STROKEDASH_LENGTH = 8
    };

    void fill_id(char* id, char* fill, int array_length);
    void fill_bounds(float* bounds, float* fill, int array_length);
    void fill_strokedash_array(float* strokedash, float* fill, int array_length);

    void fill_beziercurve(float* beziercurve,
        int array_length,
        float x1, float y1,
        float x2, float y2,
        float control_x1, float control_y1,
        float control_x2, float control_y2);

    NSVGpath* create_path(float width, float height, coordinate start, coordinate end);
    NSVGshape* create_shape(float width, float height, pixel color);
    NSVGimage* create_nsvgimage(float width, float height);

}