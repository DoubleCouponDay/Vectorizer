#pragma once

#include "chunkmap.h"

namespace vectorizer
{
    class IShapeFinder
    {
    public:
        virtual ~IShapeFinder() {}

        virtual void fill_chunkmap(chunkmap& map, float threshold) = 0;
    };
}