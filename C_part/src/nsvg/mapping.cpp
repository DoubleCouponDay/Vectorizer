#include "mapping.h"

#include <stdlib.h>
#include <nanosvg.h>

#include "mapping.h"
#include "utility/error.h"
#include "utility/logger.h"
#include "image.h"
#include "chunkmap.h"
#include "copy.h"


namespace vectorizer
{

    void fill_float_array(float* tobefilled, float* fill, int array_length, int max_length) {
        if (array_length > max_length) {
            LOG_INFO("arrays length must be less than: %d", max_length);
            setError(ARRAY_DIFF_SIZE_ERROR);
            return;
        }

        for (int i = 0; i < array_length; ++i) {
            tobefilled[i] = fill[i];
        }
    }

    void fill_strokedash_array(float* strokedash, float* fill, int array_length) {
        fill_float_array(strokedash, fill, array_length, STROKEDASH_LENGTH);

        if (isBadError()) {
            LOG_INFO("fill_float_array failed with code: %d", getLastError());
            return;
        }
    }

    void fill_id(char* id, char* fill, int array_length) {
        if (array_length > ID_LENGTH) {
            LOG_INFO("new id length must be less than: %d", BOUNDS_LENGTH);
            setError(ARRAY_DIFF_SIZE_ERROR);
            return;
        }
        int fillindex = 0;

        while (fillindex < array_length) {
            id[fillindex] = fill[fillindex];
            ++fillindex;
        }
    }

    void fill_bounds(float* bounds, float* fill, int array_length) {
        fill_float_array(bounds, fill, array_length, BOUNDS_LENGTH);
        int code = getLastError();

        if (isBadError()) {
            LOG_INFO("fill_float_array failed with code: %d", code);
            return;
        }
    }

    void fill_beziercurve(float* beziercurve,
        int array_length,
        float x1, float y1,
        float x2, float y2,
        float control_x1, float control_y1,
        float control_x2, float control_y2) {

        if (beziercurve == NULL) {
            LOG_INFO("array is null");
            setError(NULL_ARGUMENT_ERROR);
            return;
        }

        if (array_length != BEZIERCURVE_LENGTH) {
            LOG_INFO("beziercurve array must be 8 long.");
            setError(ARRAY_DIFF_SIZE_ERROR);
            return;
        }
        beziercurve[0] = x1;
        beziercurve[1] = y1;
        beziercurve[2] = x2;
        beziercurve[3] = y2;
        beziercurve[4] = control_x1;
        beziercurve[5] = control_y1;
        beziercurve[6] = control_x2;
        beziercurve[7] = control_y2;
    }

    NSVGpath* create_path(float width, float height, coordinate start, coordinate end) {
        NSVGpath* output = reinterpret_cast<NSVGpath*>(calloc(1, sizeof(NSVGpath)));
        float* points = reinterpret_cast<float*>(calloc(1, sizeof(float) * BEZIERCURVE_LENGTH));
        output->pts = points;
        float boundingbox[4] = { 0, 0, width, height };

        fill_beziercurve(output->pts, BEZIERCURVE_LENGTH, start.x, start.y, end.x, end.y, 0, 0, 1, 1); //draw the top side of a box
        fill_bounds(output->bounds, boundingbox, BOUNDS_LENGTH);
        int code = getLastError();

        if (isBadError()) {
            LOG_INFO("fill_bounds failed with code: %d", code);
            free(output);
            free(points);
            return NULL;
        }

        output->npts = 2;
        output->closed = 1;
        output->next = NULL;
        return output;
    }

    NSVGshape* create_shape(float width, float height, pixel color) {
        NSVGshape* output = reinterpret_cast<NSVGshape*>(calloc(1, sizeof(NSVGshape)));

        NSVGpaint fill = {
            NSVG_PAINT_COLOR,
            NSVG_RGB(color.R, color.G, color.B)
        };
        output->fill = fill;

        NSVGpaint stroke = {
            NSVG_PAINT_NONE,
            NSVG_RGB(color.R, color.G, color.B)
        };
        output->stroke = stroke;
        output->opacity = 1.0;
        output->strokeWidth = 0.0;
        output->strokeDashOffset = 0.0;

        float strokedash[1] = { 0 };
        char strokeDashCount = 1;
        fill_strokedash_array(output->strokeDashArray, strokedash, strokeDashCount); //idk if we need this
        int code = getLastError();

        if (isBadError()) {
            free(output);
            LOG_INFO("fill_strokedash_array failed with code: %d", code);
            return nullptr;
        }
        output->strokeDashCount = strokeDashCount;
        output->strokeLineJoin = NSVG_JOIN_MITER;
        output->strokeLineCap = NSVG_CAP_BUTT;
        output->miterLimit = 0;
        output->fillRule = NSVG_FILLRULE_NONZERO;
        output->flags = NSVG_FLAGS_VISIBLE;

        float newbounds[BOUNDS_LENGTH] = {
            0, 0,
            width,
            height
        };
        fill_bounds(output->bounds, newbounds, BOUNDS_LENGTH);

        if (isBadError()) {
            free(output);
            LOG_INFO("fill_bounds failed with: %d", getLastError());
            return nullptr;
        }

        output->paths = NULL;
        output->next = NULL;
        return output;
    }

    NSVGimage* create_nsvgimage(float width, float height) {
        NSVGimage* output = reinterpret_cast<NSVGimage*>(calloc(1, sizeof(NSVGimage)));
        output->width = width;
        output->height = height;
        output->shapes = NULL;
        return output;
    }

}