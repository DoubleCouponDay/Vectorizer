#include "pixel.h"

#include <math.h>

#include "vec.h"

namespace vectorizer
{
    bool pixel::is_similar_to(const pixel& other, float threshold)
    {
        return pixelInt(*this).is_similar_to(other, threshold);
    }

    bool pixelInt::is_similar_to(const pixelInt& other, float threshold)
    {
        pixelInt diff{ R - other.R, G - other.G, B - other.B };

        float mag = sqrtf((int)(diff.R * diff.R + diff.G * diff.G + diff.B * diff.B));

        return mag <= threshold;
    }

    pixelD pixelD::lerp(pixelD a, pixelD b, float t)
    {
        return pixelD(::lerp(a.R, b.R, t), ::lerp(a.G, b.G, t), ::lerp(a.B, b.B, t));
    }

    pixelF pixelF::lerp(pixelF a, pixelF b, float t)
    {
        return pixelF(::lerp(a.R, b.R, t), ::lerp(a.G, b.G, t), ::lerp(a.B, b.B, t));
    }

    bool pixelF::is_similar_to(const pixelF& other, float threshold)
    {
        pixelF diff{ R - other.R, G - other.G, B - other.B };

        float mag = sqrtf((diff.R * diff.R) + (diff.G * diff.G) + (diff.B * diff.B));

        return mag <= (threshold / (441.f / 3.f));
    }
}