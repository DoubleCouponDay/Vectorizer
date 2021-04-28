#pragma once

#include <stdlib.h>
#include <nanosvg.h>

#include "chunkmap.h"
#include "nsvg/usage.h"

namespace vectorizer
{
	bool write_svg_file(const nsvg_ptr& input);

	constexpr char* OUTPUT_PATH = "output.svg";
}