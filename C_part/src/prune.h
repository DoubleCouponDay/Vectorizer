#pragma once

#include <list>
#include <memory>

#include "chunkmap.h"

namespace vectorizer
{
	bool chunk_is_adjacent(pixelchunk& current, pixelchunk& subject);
	void prune_boundary(std::list<std::shared_ptr<pixelchunk>> boundary);
}
