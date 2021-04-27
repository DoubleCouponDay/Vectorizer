#pragma once

#include <nanosvg.h>
#include "chunkmap.h"

#include "ishapefinder.h"

void fill_chunkmap(chunkmap& map, const vectorize_options& options);

class dcdfiller : public IShapeFinder
{
public:
	void fill_chunkmap(chunkmap& map, float threshold) override;
};