#pragma once

#include "image.h"

namespace vectorizer
{
	class reducer
	{
	public:
		reducer(size_t reach, float threshold, float similarity = 20.f);

		Image reduce_image(const Image& from) const;

	private:

		static Image averages_of(const Image& from, size_t reach);

		static Bitmap<size_t> find_similar_neighbour_averages(const Image& averages, size_t reach, float similarity);

		static Image most_popular_neighbours(const Image& from, const Image& averages, const Bitmap<size_t>& counts, size_t reach, float threshold);

		size_t _reach;
		float _threshold;
		float _similarity;
	};
}