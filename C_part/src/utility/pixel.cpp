#include "pixel.h"

#include <math.h>

#include "vec.h"

namespace vectorizer
{
    bool pixel::is_similar_to(const pixel& other, float threshold) const
    {
        return pixelInt(*this).is_similar_to(other, threshold);
    }

    float pixel::difference_from(const pixel& other) const
    {
        return pixelInt(*this).difference_from(other);
    }

    bool pixelInt::is_similar_to(const pixelInt& other, float threshold)
    {
        pixelInt diff{ R - other.R, G - other.G, B - other.B };

        float mag = sqrtf((float)(diff.R * diff.R + diff.G * diff.G + diff.B * diff.B));

        return mag <= threshold;
    }

    float pixelInt::difference_from(const pixelInt& other) const
    {
        pixelInt diff{ R - other.R, G - other.G, B - other.B };

        float R = (float)diff.R, G = (float)diff.G, B = (float)diff.B;

        float mag = sqrtf(R * R + G * G + B * B);

        return mag;
    }

    pixelD pixelD::lerp(pixelD a, pixelD b, float t)
    {
        return pixelD(vectorizer::lerp(a.R, b.R, t), vectorizer::lerp(a.G, b.G, t), vectorizer::lerp(a.B, b.B, t));
    }

    pixelF pixelF::lerp(pixelF a, pixelF b, float t)
    {
        return pixelF(vectorizer::lerp(a.R, b.R, t), vectorizer::lerp(a.G, b.G, t), vectorizer::lerp(a.B, b.B, t));
    }

    bool pixelF::is_similar_to(const pixelF& other, float threshold)
    {
        pixelF diff{ R - other.R, G - other.G, B - other.B };

        float mag = sqrtf((diff.R * diff.R) + (diff.G * diff.G) + (diff.B * diff.B));

        return mag <= (threshold / (441.f / 3.f));
    }
}