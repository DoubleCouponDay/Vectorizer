#pragma once

#include "chunkmap.h"

class IShapeFinder
{
public:
    virtual ~IShapeFinder() {}

    virtual void fill_chunkmap(chunkmap& map, float threshold) = 0;
};