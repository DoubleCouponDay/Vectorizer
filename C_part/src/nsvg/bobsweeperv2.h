#pragma once

#include <string>

#include "ishapefinder.h"

class bobsweeperv2 : public IShapeFinder
{
public:
	
	void fill_chunkmap(chunkmap& map, float threshold) override;

	void process_image(chunkmap& map, float threshold, std::string svg_path);
	void process_image(const image& img, float threshold, std::string svg_path);
};