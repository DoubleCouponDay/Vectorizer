#pragma once

#include <string>

#include "ishapefinder.h"
#include "utility/serialize.h"

namespace vectorizer
{

	class bobsweeperv2 : public IShapeFinder
	{
	public:

		void fill_chunkmap(chunkmap& map, float threshold) override;

		void process_image(const IPixelAccess& pixels, float threshold, std::string svg_path);
	};

	namespace scan
	{
		class shape : public ISerializable
		{
		public:
			explicit shape() = default;
			shape(pixelF color, sizei image_size, vector2i first_chunk);
			shape(std::istream& read_stream);
			shape(shape&& other) = default;
			shape(const shape& other) = default;
			shape& operator=(const shape& other) = default;
			shape& operator=(shape&& other) = default;

			pixelF color() const;
			void set_color(pixelF color);

			const bounds2di& bounds() const;
			const size_t& chunk_count() const;
			const std::vector<bool>& chunks() const;
			const std::vector<vector2>& outer_edge() const;
			const std::vector<vector2i>& outer_edge_points() const;
			constexpr dimensional_indexer indexer() const { return dimensional_indexer{ _chunks_size.x, _chunks_size.y }; }
			bool has(vector2i spot) const;


			void insert_chunk(vector2i at);
			void insert_outer_edge(vector2 at);
			void insert_outer_edge_point(vector2i at);

			void compress_chunks();

			virtual void Serialize(std::ostream& write_stream) const override;
			virtual void Deserialize(std::istream& read_stream) override;

		private:

			vector2i _chunks_offset;
			std::vector<bool> _chunks;
			std::vector<vector2> _outer_edge;
			std::vector<vector2i> _outer_edge_points;

			pixelF _color;
			bounds2di _bounds;
			sizei _chunks_size;
			size_t _chunk_count;
		};

		class pixel_scan : public ISerializable
		{
		public:
			pixel_scan(const IPixelAccess& pixels);
			pixel_scan(std::istream& read_stream);

			constexpr const std::vector<shape>& shapes() const { return _shapes; }
			constexpr dimensional_indexer indexer() const { return dimensional_indexer{ _image_size.x }; }
			
			bool has_shape(vector2i spot) const;

			std::vector<vector2i> neighbours_of(vector2i spot) const;

			bool is_outside_image(vector2i spot) const;
			bool is_inside_image(vector2i spot) const;


			void scan_for_shapes(float threshold);
			void average_colours();
			void calculate_borders();

			void shapes_to_png(std::string png_path);
			void shape_to_png(size_t shape, std::string png_path);
			void border_to_png(size_t border, std::string png_path);
			void borders_to_png(std::string png_path);
			void to_svg(std::string svg_path);

			void compress_shapes();

			virtual void Serialize(std::ostream& write_stream) const override;
			virtual void Deserialize(std::istream& read_stream) override;

		private:
			pixel_scan() = default;
			pixel_scan& operator=(pixel_scan && other) = default;
			pixel_scan& operator=(const pixel_scan & other) = default;

			bool _has_borders;
			sizei _image_size;

			std::vector<shape> _shapes;

			std::vector<pixelF> _image;
			std::vector<int> _image_shapes;

		};
	}

}