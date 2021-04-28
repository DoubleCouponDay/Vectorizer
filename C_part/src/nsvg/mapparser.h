#pragma once
#include <nanosvg.h>
#include "../chunkmap.h"
#include "usage.h"

namespace vectorizer
{
	void parse_map_into_nsvgimage(const chunkmap& map, nsvg_ptr& output);
}
