#pragma once

#include <string>

#include "ishapefinder.h"

namespace vectorizer
{

	class bobsweeperv2 : public IShapeFinder
	{
	public:

		void fill_chunkmap(chunkmap& map, float threshold) override;

		void process_image(chunkmap& map, float threshold, std::string svg_path);
		void process_image(const Image& img, float threshold, std::string svg_path);
	};

	namespace scan
	{
		class shape
		{
		public:
			shape(pixelF color, sizei image_size, vector2i first_chunk);
			shape& operator=(const shape& other) = default;

			inline pixelF color() const { return _color; }
			inline void set_color(pixelF color) { _color = color; }

			inline const bounds2di& bounds() const { return _bounds; }
			inline const size_t& chunk_count() const { return _chunk_count; }
			inline const std::vector<bool>& chunks() const { return _chunks; }
			inline const std::vector<vector2>& outer_edge() const { return _outer_edge; }
			inline const std::vector<vector2i>& outer_edge_points() const { return _outer_edge_points; }
			inline constexpr dimensional_indexer indexer() const { return { _image_size.x }; }
			inline bool has(vector2i spot) const { if (spot.x < 0 || spot.y < 0 || spot.x >= _image_size.x || spot.y >= _image_size.y) return false; return _chunks[indexer()(spot)]; }

			inline void insert_chunk(vector2i at)
			{
				if (at.x < _bounds.min.x)
					_bounds.min.x = at.x;
				if (at.y < _bounds.min.y)
					_bounds.min.y = at.y;
				if (at.x > _bounds.max.x)
					_bounds.max.x = at.x;
				if (at.y > _bounds.max.y)
					_bounds.max.y = at.y;

				_chunks[indexer()(at.x, at.y)] = true;
				++_chunk_count;
			}

			inline void insert_outer_edge(vector2 at)
			{
				_outer_edge.push_back(at);
			}

			inline void insert_epico(vector2i at)
			{
				_outer_edge_thingo.push_back(at);
			}

		private:
			std::vector<bool> _chunks;
			std::vector<vector2> _outer_edge;
			std::vector<vector2i> _outer_edge_points;

			pixelF _color;
			bounds2di _bounds;
			sizei _image_size;
			size_t _chunk_count;
		};

		class pixel_scan
		{
		public:



		private:


		};
	}

}