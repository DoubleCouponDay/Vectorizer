#pragma once

#include "ishapefinder.h"

void sweepfill_chunkmap(chunkmap& map, float threshold);

class bobsweeper : public IShapeFinder
{
public:
    void fill_chunkmap(chunkmap& map, float threshold) override;
};