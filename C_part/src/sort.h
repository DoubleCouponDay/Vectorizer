#pragma once

#include "chunkmap.h"

namespace vectorizer
{

	void bubble_sort(std::vector<std::shared_ptr<pixelchunk>>& array, unsigned long start, unsigned long length);
	void sort_boundary(chunkmap& map);

}