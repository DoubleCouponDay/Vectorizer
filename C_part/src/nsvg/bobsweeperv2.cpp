#include "bobsweeperv2.h"

#include <memory>
#include <stdexcept>
#include <algorithm>

#include "utility/vec.h"
#include "utility/pixel.h"
#include "utility/logger.h"

// SVG Writing stuff
#include <fstream>
#include <nanosvg.h>

#include "nsvg/copy.h"

class shape
{
	pixelF _color;
	bounds2di _bounds;
	
	size_t _chunk_count;
	sizei _image_size;
	std::vector<bool> _chunks; // size of image
	std::vector<vector2> _outer_edge;
	std::vector<vector2i> _outer_edge_thingo;

public:
	shape(pixelF color, sizei image_size, vector2i first_chunk) : 
		_color(color), 
		_image_size(image_size),
		_chunks(image_size.x * image_size.y), 
		_bounds(first_chunk.x, first_chunk.y, first_chunk.x, first_chunk.y),
		_chunk_count(1)
	{
		_chunks[indexer()(first_chunk.x, first_chunk.y)] = true;
		_bounds.max.x = _bounds.min.x = first_chunk.x;
		_bounds.max.y = _bounds.min.y = first_chunk.y;
	}

	shape(const shape& other) = default;
	shape(shape&& other) :
		_color(other._color),
		_bounds(other._bounds),
		_chunk_count(other._chunk_count),
		_image_size(other._image_size),
		_chunks(std::move(other._chunks)),
		_outer_edge(std::move(other._outer_edge)),
		_outer_edge_thingo(std::move(other._outer_edge_thingo))
	{
		other._color = pixelF{ 0.f, 0.f, 0.f };
		other._bounds = bounds2di{ {0, 0}, {0, 0} };
		other._chunk_count = 0;
		other._image_size = sizei{ 0,0 };
		other._chunks.clear();
		other._outer_edge.clear();
		other._outer_edge_thingo.clear();
	}

	shape& operator=(const shape& other) = default;

	inline pixelF color() const { return _color; }
	inline void set_color(pixelF color) { _color = color; }

	inline const bounds2di& bounds() const { return _bounds; }
	inline const size_t& chunk_count() const { return _chunk_count; }
	inline const std::vector<bool>& chunks() const { return _chunks; }
	inline const std::vector<vector2>& outer_edge() const { return _outer_edge; }
	inline const std::vector<vector2i>& thingo() const { return _outer_edge_thingo; }
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
};

class thingo
{
	std::vector<shape> _shapes;

	std::vector<pixelF> _image;
	std::vector<int> _image_shapes;
	sizei _image_size;

	vector2i first_empty_spot()
	{
		vector2i pos = { 0, 0 };

		auto index = indexer();
		for ( ; pos.x < pos.x; ++pos.x)
		{
			for ( ; pos.y < pos.y; ++pos.y)
			{
				if (_image_shapes[index(pos.x, pos.y)] == -1)
				{
					return pos;
				}
			}
		}
	}

	std::vector<vector2i> neighbours_of(vector2i spot)
	{
		return { 
			{ spot.x - 1,	spot.y - 1	}, 
			{ spot.x	,	spot.y - 1	}, 
			{ spot.x + 1,	spot.y - 1	}, 
			{ spot.x - 1,	spot.y		}, 
			{ spot.x + 1,	spot.y		}, 
			{ spot.x - 1,	spot.y + 1	}, 
			{ spot.x	,	spot.y + 1	}, 
			{ spot.x + 1,	spot.y + 1	} 
		};
	}

	inline bool is_outside_image(vector2i spot)
	{
		return (spot.x < 0) | (spot.y < 0) | (spot.x >= _image_size.x) | (spot.y >= _image_size.y);
	}

	inline bool is_inside_image(vector2i spot)
	{
		return (spot.x >= 0) & (spot.y >= 0) & (spot.x < _image_size.x) & (spot.y < _image_size.y);
	}

public:
	thingo(const chunkmap& map) : 
		_image_size(map.width(), map.height()), 
		_image(map.width() * map.height()), 
		_image_shapes(map.width() * map.height())
	{
		if (map.width() < 1 || map.height() < 1)
			throw std::invalid_argument("Can not accept chunkmap with no chunks in it");

		auto index = indexer();
		for (int x = 0; x < map.width(); ++x)
			for (int y = 0; y < map.height(); ++y)
			{
				_image[index(x, y)] = map.get(x, y)->average_colour;
			}
	}

	thingo(const image& img) :
		_image_size(img.width(), img.height()),
		_image(img.width()* img.height()),
		_image_shapes(img.width()* img.height())
	{
		if (img.width() < 1 || img.height() < 1)
			throw std::invalid_argument("Can not accept image with no pixels in it");

		auto index = indexer();
		for (int x = 0; x < img.width(); ++x)
		{
			for (int y = 0; y < img.height(); ++y)
			{
				_image[index(x, y)] = img.get(x, y);
			}
		}
	}

	inline constexpr const std::vector<shape>& shapes() const { return _shapes; }
	inline constexpr dimensional_indexer indexer() const { return { _image_size.x }; }

	inline bool has_shape(vector2i spot)
	{
		return is_inside_image(spot) && _image_shapes[indexer()(spot.x, spot.y)] != -1;
	}

	void scan_for_shapes(float threshold)
	{
		_shapes.clear();

		for (int i = 0; i < _image_shapes.size(); ++i)
			_image_shapes[i] = -1;

		auto index = indexer();

		std::vector<vector2i> to_check{};

		int iter = 0;
		while (std::any_of(_image_shapes.begin(), _image_shapes.end(), [](int i) { return i == -1; }))
		{
			for (int i = 0; i < _image.size(); ++i)
				if (_image_shapes[i] == -1)
				{
					_image_shapes[i] = _shapes.size();
					_shapes.emplace_back(_image[i], _image_size, index.reverse(i));
					to_check.emplace_back(index.reverse(i));
					break;
				}
			if (iter++ > 5000)
			{
				LOG_WARN("Killing Shape Search: iterations hit 1000 (intended?)");
				break;
			}
			while (to_check.size())
			{
				vector2i spot = { -1, -1 };
				int spot_index = 0;
				for ( ; spot_index < to_check.size(); ++spot_index)
					if (has_shape(to_check[spot_index]))
					{
						spot = to_check[spot_index];
						break;
					}

				if (spot.x == -1 || spot.y == -1)
					break;

				std::vector<vector2i> neighbours = neighbours_of(spot);
				for (auto& neighbour : neighbours)
				{
					if (is_outside_image(neighbour))
					{

					}
					else
					{
						if (!has_shape(neighbour) && _image[index(neighbour)].is_similar_to(_image[index(spot)], threshold))
						{
							to_check.push_back(neighbour);
							_shapes.back().insert_chunk(neighbour);
							_image_shapes[index(neighbour)] = _image_shapes[index(spot)];
						}
					}
				}

				std::swap(to_check[spot_index], to_check.back());
				to_check.pop_back();
			}
		}

		LOG_INFO("[BobSweeperV2] Scanned %u shapes", _shapes.size());
	}

	void average_colors()
	{
		auto index = indexer();
		for (auto& shape : _shapes)
		{
			int total = shape.chunk_count();

			pixelF sum{ 0.f, 0.f, 0.f };

			for (int x = shape.bounds().min.x; x <= shape.bounds().max.x; ++x)
			{
				for (int y = shape.bounds().min.y; y <= shape.bounds().max.y; ++y)
				{
					if (shape.has(vector2i{ x, y }))
						sum += _image[index(x, y)];
				}
			}

			pixelF average = sum / (float)total;

			shape.set_color(average);
		}
	}

	void calculate_borders()
	{
		auto index = indexer();
		for (auto& shape : _shapes)
		{
			CompassDirection first_dir = CompassDirection::North, edge = CompassDirection::North;
			vector2i first, spot;

			// Find any piece on the outer edge and follow it around
			for (int x = shape.bounds().min.x; x <= shape.bounds().max.x; ++x)
			{
				if (shape.has(vector2i{ x, shape.bounds().min.y }))
				{
					first = spot = vector2i{ x, shape.bounds().min.y - 1 };
						
					break;
				}
			}

			int iter = 0;
			int max_iter = shape.chunk_count() * 4 + 1;
			while (iter++ < max_iter)
			{
				// TODO: Check if next spot if the original spot
				if (shape.has(spot + edge.turned_by(1).to_diri()))
				{
					// Insert current edge
					shape.insert_outer_edge((vector2)spot + edge.to_dir() * 0.5f);
					shape.insert_epico(spot + edge.to_diri());

					// Set next edge and spot
					spot;
					edge = edge.turned_by(1);

					if (spot == first && edge == first_dir)
						break;

					continue;
				}

				if (shape.has(spot + edge.turned_by(1).to_diri() + edge.to_diri()))
				{
					// Insert current edge
					shape.insert_outer_edge((vector2)spot + edge.to_dir() * 0.5f);
					shape.insert_epico(spot + edge.to_diri());

					// Set next edge and spot
					spot = spot + edge.turned_by(1).to_diri();
					edge;

					if (spot == first && edge == first_dir)
						break;

					continue;
				}

				// Insert current edge
				shape.insert_outer_edge((vector2)spot + edge.to_dir() * 0.5f);
				shape.insert_epico(spot + edge.to_diri());

				// Set next edge and spot
				spot = spot + edge.turned_by(1).to_diri() + edge.to_diri();
				edge = edge.turned_by(-1);

				if (spot == first && edge == first_dir)
					break;
			}

		}
	}
};

void bobsweeperv2::fill_chunkmap(chunkmap& map, float threshold)
{
	thingo thing = thingo(map);

	thing.scan_for_shapes(40.f);

	thing.calculate_borders();

	image img{ map.width(), map.height() };

	for (int i = 0; i < thing.shapes().size(); ++i)
	{
		auto& s = thing.shapes()[i];

		for (auto& edge : s.thingo())
		{
			img.set(edge.x, edge.y, s.color());
		}
	}

	img.to_png("epic borders.png");

	image shape_img{ map.width(), map.height() };
	for (auto& shape : thing.shapes())
	{
		for (int x = shape.bounds().min.x; x <= shape.bounds().max.x; ++x)
		{
			for (int y = shape.bounds().min.y; y < shape.bounds().max.y; ++y)
			{
				if (shape.has(vector2i{ x,y }))
					shape_img.set(x, y, shape.color());
			}
		}
	}

	shape_img.to_png("epic shapes.png");

	LOG_INFO("[BobSweeperv2] Found %u shapes", thing.shapes().size());


}

void bobsweeperv2::process_image(chunkmap& map, float threshold, std::string svg_path)
{
	LOG_INFO("[BobSweeperV2] with threshold: %.1f", threshold);

	thingo thing = thingo(map);

	thing.scan_for_shapes(threshold);
	//thing.average_colors();
	thing.calculate_borders();

	image border_img{ map.width(), map.height() };

	for (int i = 0; i < thing.shapes().size(); ++i)
	{
		auto& s = thing.shapes()[i];

		for (auto& edge : s.thingo())
		{
			border_img.set(edge.x, edge.y, s.color());
		}
	}

	border_img.to_png("epic borders.png");

	image shape_img = std::move(border_img);
	shape_img.clear();
	for (auto& shape : thing.shapes())
	{
		for (int x = shape.bounds().min.x; x <= shape.bounds().max.x; ++x)
		{
			for (int y = shape.bounds().min.y; y <= shape.bounds().max.y; ++y)
			{
				if (shape.has(vector2i{ x,y }))
					shape_img.set(x, y, shape.color());
			}
		}
	}

	shape_img.to_png("epic shapes.png");


	std::vector<shape> sorted_shapes = thing.shapes();

	struct
	{
		bool operator()(const shape& lhs, const shape& rhs) const { return lhs.bounds().area() > rhs.bounds().area(); }
	} customLess;

	std::sort(sorted_shapes.begin(), sorted_shapes.end(), customLess);

	std::ofstream out_svg{ svg_path };

	if (!out_svg)
	{
		LOG_ERR("[BobSweeperV2] Failed to open '%s' for writing.. :/", svg_path.c_str());
		throw std::invalid_argument("Unusable file path to write to");
	}

	out_svg << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?> <svg xmlns = \"http://www.w3.org/2000/svg\" xmlns:xlink = \"http://www.w3.org/1999/xlink\" version = \"2\" width = \"";
	out_svg << (float)map.width();
	out_svg << "\" height = \"";
	out_svg << (float)map.height();
	out_svg << "\" viewport = \"0 0 " << (float)map.width() << " " << (float)map.height() << "\" >";
	out_svg << std::endl;

	for (auto& shape : sorted_shapes)
	{
		out_svg << "<path fill=\"rgb(" << shape.color().R * 255.f << ", " << shape.color().G * 255.f << ", " << shape.color().B * 255.f << ")\" d=\"";
		out_svg << "M " << shape.outer_edge()[0].x << " " << shape.outer_edge()[0].y << " ";
		for (int i = 1; i < shape.outer_edge().size(); ++i)
		{
			out_svg << " L " << shape.outer_edge()[i].x << " " << shape.outer_edge()[i].y;
		}
		out_svg << " Z\" />" << std::endl;
	}

	out_svg << " </svg>";
}

void bobsweeperv2::process_image(const image& img, float threshold, std::string svg_path)
{
	LOG_INFO("[BobSweeperV2] with threshold: %.1f", threshold);

	thingo thing = thingo(img);

	thing.scan_for_shapes(threshold);
	thing.average_colors();
	thing.calculate_borders();

	image border_img{ img.width(), img.height() };

	for (int i = 0; i < thing.shapes().size(); ++i)
	{
		auto& s = thing.shapes()[i];

		for (auto& edge : s.thingo())
		{
			border_img.set(edge.x, edge.y, s.color());
		}
	}

	border_img.to_png("epic borders.png");

	image shape_img = std::move(border_img);
	shape_img.clear();
	for (auto& shape : thing.shapes())
	{
		for (int x = shape.bounds().min.x; x <= shape.bounds().max.x; ++x)
		{
			for (int y = shape.bounds().min.y; y <= shape.bounds().max.y; ++y)
			{
				if (shape.has(vector2i{ x,y }))
					shape_img.set(x, y, shape.color());
			}
		}
	}

	shape_img.to_png("epic shapes.png");


	std::vector<shape> sorted_shapes = thing.shapes();

	struct 
	{
		bool operator()(const shape& lhs, const shape& rhs) const { return lhs.bounds().area() < rhs.bounds().area(); }
	} customLess;

	std::sort(sorted_shapes.begin(), sorted_shapes.end(), customLess);

	std::ofstream out_svg{ svg_path };

	if (!out_svg)
	{
		LOG_ERR("[BobSweeperV2] Failed to open '%s' for writing.. :/", svg_path.c_str());
		throw std::invalid_argument("Unusable file path to write to");
	}

	out_svg << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?> <svg xmlns = \"http://www.w3.org/2000/svg\" xmlns:xlink = \"http://www.w3.org/1999/xlink\" version = \"2\" width = \"";
	out_svg << (float)img.width();
	out_svg << "\" height = \"";
	out_svg << (float)img.height();
	out_svg << "\" viewport = \"0 0 " << (float)img.width() << " " << (float)img.height() << "\" >";
	out_svg << std::endl;

	for (auto& shape : sorted_shapes)
	{
		out_svg << "<path fill=\"rgb(" << shape.color().R << ", " << shape.color().G << ", " << shape.color().B << ")\" d=\"";
		out_svg << "M " << shape.outer_edge()[0].x << " " << shape.outer_edge()[0].y << " ";
		for (int i = 1; i < shape.outer_edge().size(); ++i)
		{
			out_svg << " L " << shape.outer_edge()[i].x << " " << shape.outer_edge()[i].y;
		}
		out_svg << " Z\" />" << std::endl;
	}

	out_svg << " </svg>";
}