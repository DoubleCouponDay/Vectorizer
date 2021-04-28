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

namespace vectorizer
{
	void bobsweeperv2::fill_chunkmap(chunkmap& map, float threshold)
	{
		scan::pixel_scan scan = scan::pixel_scan(map);

		scan.scan_for_shapes(threshold);

		scan.calculate_borders();

		Image img{ map.width(), map.height() };

		for (int i = 0; i < scan.shapes().size(); ++i)
		{
			auto& s = scan.shapes()[i];

			for (auto& edge : s.outer_edge_points())
			{
				img.set(edge.x, edge.y, s.color());
			}
		}

		img.to_png("epic borders.png");

		Image shape_img{ map.width(), map.height() };
		for (auto& shape : scan.shapes())
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

		LOG_INFO("[BobSweeperv2] Found %u shapes", scan.shapes().size());


	}

	void bobsweeperv2::process_image(const IPixelAccess& pixels, float threshold, std::string svg_path)
	{
		LOG_INFO("[BobSweeperV2] with threshold: %.1f", threshold);

		scan::pixel_scan thing = scan::pixel_scan(pixels);

		thing.scan_for_shapes(threshold);
		//thing.average_colors();
		thing.calculate_borders();

		Image border_img{ (size_t)pixels.get_width(), (size_t)pixels.get_height() };

		for (int i = 0; i < thing.shapes().size(); ++i)
		{
			auto& s = thing.shapes()[i];

			for (auto& edge : s.outer_edge_points())
			{
				border_img.set(edge.x, edge.y, s.color());
			}
		}

		border_img.to_png("epic borders.png");

		Image shape_img = std::move(border_img);
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


		std::vector<scan::shape> sorted_shapes = thing.shapes();

		struct
		{
			bool operator()(const scan::shape& lhs, const scan::shape& rhs) const { return lhs.bounds().area() > rhs.bounds().area(); }
		} customLess;

		std::sort(sorted_shapes.begin(), sorted_shapes.end(), customLess);

		std::ofstream out_svg{ svg_path };

		if (!out_svg)
		{
			LOG_ERR("[BobSweeperV2] Failed to open '%s' for writing.. :/", svg_path.c_str());
			throw std::invalid_argument("Unusable file path to write to");
		}

		out_svg << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?> <svg xmlns = \"http://www.w3.org/2000/svg\" xmlns:xlink = \"http://www.w3.org/1999/xlink\" version = \"2\" width = \"";
		out_svg << (float)pixels.get_width();
		out_svg << "\" height = \"";
		out_svg << (float)pixels.get_height();
		out_svg << "\" viewport = \"0 0 " << (float)pixels.get_width() << " " << (float)pixels.get_height() << "\" >";
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

	namespace scan
	{
		shape::shape(pixelF color, sizei image_size, vector2i first_chunk) :
			_color(color),
			_chunks_size(image_size),
			_chunks(image_size.x* image_size.y),
			_bounds(first_chunk.x, first_chunk.y, first_chunk.x, first_chunk.y),
			_chunk_count(1),
			_chunks_offset{ 0, 0 }
		{
			_chunks[indexer()(first_chunk)] = true;
		}

		shape::shape(std::istream& read_stream)
		{
			this->shape::Deserialize(read_stream);
		}

		pixelF shape::color() const
		{
			return _color;
		}

		void shape::set_color(pixelF color)
		{
			_color = color;
		}

		const bounds2di& shape::bounds() const
		{
			return _bounds;
		}

		const size_t& shape::chunk_count() const
		{
			return _chunk_count;
		}

		const std::vector<bool>& shape::chunks() const
		{
			return _chunks;
		}

		const std::vector<vector2>& shape::outer_edge() const
		{
			return _outer_edge;
		}

		const std::vector<vector2i>& shape::outer_edge_points() const
		{
			return _outer_edge_points;
		}

		bool shape::has(vector2i spot) const
		{
			spot += _chunks_offset;
			if (spot.x < 0 || spot.y < 0 || spot.x >= _chunks_size.x || spot.y >= _chunks_size.y) 
				return false; 
			return _chunks[indexer()(spot)];
		}

		void shape::insert_chunk(vector2i at)
		{
			if (_chunks_offset != vector2i{ 0, 0 })
				throw std::logic_error("Inserting chunks into a compressed chunk is not implemented");
			if (at.x < _bounds.min.x)
				_bounds.min.x = at.x;
			if (at.y < _bounds.min.y)
				_bounds.min.y = at.y;
			if (at.x > _bounds.max.x)
				_bounds.max.x = at.x;
			if (at.y > _bounds.max.y)
				_bounds.max.y = at.y;

			_chunks[indexer()(at)] = true;
			++_chunk_count;
		}

		void shape::insert_outer_edge(vector2 at)
		{
			_outer_edge.push_back(at);
		}

		void shape::insert_outer_edge_point(vector2i at)
		{
			_outer_edge_points.push_back(at);
		}

		void shape::compress_chunks()
		{
			if (_chunks_offset != vector2i{0, 0})
				return;

			std::vector<bool> new_chunks = std::vector<bool>();
			new_chunks.resize((_bounds.width() + 1) * (_bounds.height() + 1));
			dimensional_indexer indexer{ (size_t)_bounds.width() + 1 };

			for (int x = _bounds.min.x; x <= _bounds.max.x; ++x)
			{
				for (int y = _bounds.min.y; y <= _bounds.max.y; ++y)
				{
					new_chunks[indexer(x - _bounds.min.x, y - _bounds.min.y)] = _chunks[this->indexer()(x, y)];
				}
			}

			_chunks_size.x = _bounds.width() + 1;
			_chunks_size.y = _bounds.height() + 1;

			_chunks_offset = -_bounds.min;

			_chunks = std::move(new_chunks);
		}


		void shape::Serialize(std::ostream& write_stream) const
		{
			// write with format: 
			// v1.1
			// [R] [G] [B]
			// [Min Bounds X] [Min Bounds Y] [Max Bounds X] [Max Bounds Y]
			// [Image Width] [Image Height]
			// [Chunk Offset X] [Chunk Offset Y]
			// [Chunk Count]
			// [_chunks.size()]
			// [Chunks]
			// [Border Size]
			// [Border]
			// [Border Points Size]
			// [Border Points]

			write_stream << "v1.1" << std::endl;
			write_stream << _color.R << " " << _color.G << " " << _color.B << std::endl;
			write_stream << _bounds.min.x << " " << _bounds.min.y << " " << _bounds.max.x << " " << _bounds.max.y << std::endl;
			write_stream << _chunks_size.x << " " << _chunks_size.y << std::endl;
			write_stream << _chunks_offset.x << " " << _chunks_offset.y << std::endl;
			write_stream << _chunk_count << std::endl;

			write_stream << _chunks.size() << std::endl;
			for (const auto& chunk : _chunks)
			{
				write_stream << chunk << " ";
			}
			write_stream << std::endl;

			write_stream << _outer_edge.size() << std::endl;
			for (const auto& edge : _outer_edge)
			{
				write_stream << edge.x << " " << edge.y << " ";
			}
			write_stream << std::endl;

			write_stream << _outer_edge_points.size() << std::endl;
			for (const auto& edge : _outer_edge_points)
			{
				write_stream << edge.x << " " << edge.y << " ";
			}
			write_stream << std::endl;
		}

		void shape::Deserialize(std::istream& read_stream)
		{
			std::string version;
			read_stream >> version;
			if (version != "v1.1")
				throw std::invalid_argument("shape version was not v1.1");

			shape deserialized;
			read_stream >> deserialized._color.R >> deserialized._color.G >> deserialized._color.B;
			read_stream >> deserialized._bounds.min.x >> deserialized._bounds.min.y >> deserialized._bounds.max.x >> deserialized._bounds.max.y;
			read_stream >> deserialized._chunks_size.x >> deserialized._chunks_size.y;
			read_stream >> deserialized._chunks_offset.x >> deserialized._chunks_offset.y;
			read_stream >> deserialized._chunk_count;
			
			size_t chunk_count = 0;
			read_stream >> chunk_count;
			deserialized._chunks.resize(chunk_count);

			for (int i = 0; i < chunk_count; ++i)
			{
				bool val;
				read_stream >> val;
				deserialized._chunks[i] = val;
			}

			size_t edge_count = 0;
			read_stream >> edge_count;
			deserialized._outer_edge.resize(edge_count);
			
			for (int i = 0; i < edge_count; ++i)
			{
				vector2 edge;
				read_stream >> edge.x >> edge.y;
				deserialized._outer_edge[i] = edge;
			}

			size_t edge_point_count = 0;
			read_stream >> edge_point_count;
			deserialized._outer_edge_points.resize(edge_point_count);

			for (int i = 0; i < edge_point_count; ++i)
			{
				vector2i edge_spot;
				read_stream >> edge_spot.x >> edge_spot.y;
				deserialized._outer_edge_points[i] = edge_spot;
			}

			*this = std::move(deserialized);
		}



		pixel_scan::pixel_scan(const IPixelAccess& pixels) :
			_image_size(pixels.get_width(), pixels.get_height()),
			_image(pixels.get_width() * pixels.get_height()),
			_image_shapes(pixels.get_width() * pixels.get_height(), -1)
		{
			if (pixels.get_width() < 1 || pixels.get_height() < 1)
				throw std::invalid_argument("Can not accept pixels with no chunks in it");

			auto index = indexer();
			for (int x = 0; x < pixels.get_width(); ++x)
			{
				for (int y = 0; y < pixels.get_height(); ++y)
				{
					_image[index(x, y)] = pixels.get_pixel(x, y);
				}
			}
		}

		pixel_scan::pixel_scan(std::istream& read_stream)
		{
			this->pixel_scan::Deserialize(read_stream);
		}

		bool pixel_scan::has_shape(vector2i spot) const
		{
			return is_inside_image(spot) && _image_shapes[indexer()(spot.x, spot.y)] != -1;
		}

		std::vector<vector2i> pixel_scan::neighbours_of(vector2i spot) const 
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

		bool pixel_scan::is_outside_image(vector2i spot) const
		{
			return (spot.x < 0) | (spot.y < 0) | (spot.x >= _image_size.x) | (spot.y >= _image_size.y);
		}

		bool pixel_scan::is_inside_image(vector2i spot) const
		{
			return (spot.x >= 0) & (spot.y >= 0) & (spot.x < _image_size.x) & (spot.y < _image_size.y);
		}



		void pixel_scan::scan_for_shapes(float threshold)
		{
			LOG_INFO("Scanning for shapes...");
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
					for (; spot_index < to_check.size(); ++spot_index)
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

		void pixel_scan::average_colours()
		{
			auto index = indexer();
			for (auto& shape : _shapes)
			{
				int total = shape.chunk_count();

				pixelD sum{ 0.f, 0.f, 0.f };

				int count = 0;
				for (int x = shape.bounds().min.x; x <= shape.bounds().max.x; ++x)
				{
					for (int y = shape.bounds().min.y; y <= shape.bounds().max.y; ++y)
					{
						if (shape.has(vector2i{ x, y }))
						{
							sum += _image[index(x, y)];
							++count;
						}
					}
				}

				pixelF average = sum / (double)total;

				shape.set_color(average);
			}
		}

		void pixel_scan::calculate_borders()
		{
			LOG_INFO("Calculating Shape Borders");
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
						shape.insert_outer_edge_point(spot + edge.to_diri());

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
						shape.insert_outer_edge_point(spot + edge.to_diri());

						// Set next edge and spot
						spot = spot + edge.turned_by(1).to_diri();
						edge;

						if (spot == first && edge == first_dir)
							break;

						continue;
					}

					// Insert current edge
					shape.insert_outer_edge((vector2)spot + edge.to_dir() * 0.5f);
					shape.insert_outer_edge_point(spot + edge.to_diri());

					// Set next edge and spot
					spot = spot + edge.turned_by(1).to_diri() + edge.to_diri();
					edge = edge.turned_by(-1);

					if (spot == first && edge == first_dir)
						break;
				}
			}
			_has_borders = true;
		}

		void pixel_scan::shapes_to_png(std::string png_path)
		{
			Image shape_img = Image{ _image_size.x, _image_size.y };
			for (auto& shape : shapes())
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

			shape_img.to_png(png_path.c_str());
		}

		void pixel_scan::shape_to_png(size_t shape_index, std::string png_path)
		{
			if (shape_index >= _shapes.size())
				return;

			Image shape_img = Image{ _image_size.x, _image_size.y };

			const auto& shape = _shapes[shape_index];

			LOG_INFO("Shape %u has color (%.2f, %.2f, %.2f) and %u chunks", shape_index, shape.color().R, shape.color().G, shape.color().B, shape.chunk_count());
			if (vector3{ shape.color().R, shape.color().G, shape.color().B }.sqr_mag() < 0.15f)
			{
				shape_img.clear(pixelF{ 0.8f, 0.8f, 0.8f });
				LOG_INFO("Background color has been inverted (its now white)");
			}
			else
				LOG_INFO("Background color is normal (its black)");

			for (int x = shape.bounds().min.x; x <= shape.bounds().max.x; ++x)
			{
				for (int y = shape.bounds().min.y; y <= shape.bounds().max.y; ++y)
				{
					if (shape.has(vector2i{ x,y }))
						shape_img.set(x, y, shape.color());
				}
			}

			shape_img.to_png(png_path.c_str());
		}

		void pixel_scan::border_to_png(size_t border, std::string png_path)
		{
			if (border >= _shapes.size())
				return;

			Image border_img{ _image_size.x, _image_size.y };

			const auto& shape = _shapes[border];

			LOG_INFO("Shape %u has color (%.2f, %.2f, %.2f) and %u chunks", border, shape.color().R, shape.color().G, shape.color().B, shape.chunk_count());
			if (vector3{ shape.color().R, shape.color().G, shape.color().B }.sqr_mag() < 0.15f)
			{
				border_img.clear(pixelF{ 0.8f, 0.8f, 0.8f });
				LOG_INFO("Shape has a very dark color, Background color has been inverted (its now white)");
			}
			else
				LOG_INFO("Shape does not have a very dark color, Background color is normal (its black)");

			for (auto& edge : shape.outer_edge_points())
			{
				border_img.set(edge.x, edge.y, shape.color());
			}

			border_img.to_png(png_path.c_str());
		}

		void pixel_scan::borders_to_png(std::string png_path)
		{
			Image border_img{ _image_size.x, _image_size.y };

			for (int i = 0; i < shapes().size(); ++i)
			{
				auto& s = shapes()[i];

				for (auto& edge : s.outer_edge_points())
				{
					border_img.set(edge.x, edge.y, s.color());
				}
			}

			border_img.to_png(png_path.c_str());
		}


		void pixel_scan::Serialize(std::ostream& write_stream) const
		{
			LOG_INFO("Serializing scan data");
			// Write format
			// v1
			// [Has Borders]
			// [Image Width] [Image Height]
			// [Shape Count]
			// [Shapes]
			// [Image Count]
			// [Image Colors]
			// [Image Shapes]

			write_stream << "v1" << std::endl;
			write_stream << _has_borders << std::endl;
			write_stream << _image_size.x << " " << _image_size.y << std::endl;
			write_stream << _shapes.size() << std::endl;
			for (const auto& shape : _shapes)
			{
				shape.Serialize(write_stream);
			}
			write_stream << _image.size() << std::endl;
			for (const auto& color : _image)
			{
				write_stream << color.R << " " << color.G << " " << color.B << " ";
			}
			write_stream << std::endl;
			for (const auto& shape_index : _image_shapes)
			{
				write_stream << shape_index << " ";
			}
		}

		void pixel_scan::Deserialize(std::istream& read_stream)
		{
			LOG_INFO("Deserializing scan data");
			std::string version;
			read_stream >> version;
			if (version != "v1")
				throw std::invalid_argument("File does not match version");

			pixel_scan deserialized;

			read_stream >> deserialized._has_borders;
			read_stream >> deserialized._image_size.x >> deserialized._image_size.y;

			int shape_count;
			read_stream >> shape_count;

			deserialized._shapes = std::vector<shape>(shape_count);
			for (int i = 0; i < shape_count; ++i)
			{
				deserialized._shapes[i].Deserialize(read_stream);
			}

			int image_count;
			read_stream >> image_count;

			deserialized._image = std::vector<pixelF>(image_count);
			for (int i = 0; i < image_count; ++i)
			{
				read_stream >> deserialized._image[i].R >> deserialized._image[i].G >> deserialized._image[i].B;
			}

			deserialized._image_shapes = std::vector<int>(image_count);
			for (int i = 0; i < image_count; ++i)
			{
				read_stream >> deserialized._image_shapes[i];
			}

			*this = std::move(deserialized);
		}

		void pixel_scan::to_svg(std::string svg_path)
		{
			if (!_has_borders)
			{
				LOG_INFO("[BobSweeperV2] data indicates no existing borders");
				calculate_borders();
			}

			std::vector<scan::shape> sorted_shapes = shapes();

			struct
			{
				bool operator()(const scan::shape& lhs, const scan::shape& rhs) const { return lhs.bounds().area() > rhs.bounds().area(); }
			} customLess;

			std::sort(sorted_shapes.begin(), sorted_shapes.end(), customLess);

			std::ofstream out_svg{ svg_path };

			if (!out_svg)
			{
				LOG_ERR("[BobSweeperV2] Failed to open '%s' for writing.. :/", svg_path.c_str());
				throw std::invalid_argument("Unusable file path to write to");
			}

			out_svg << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?> <svg xmlns = \"http://www.w3.org/2000/svg\" xmlns:xlink = \"http://www.w3.org/1999/xlink\" version = \"2\" width = \"";
			out_svg << (float)_image_size.x;
			out_svg << "\" height = \"";
			out_svg << (float)_image_size.y;
			out_svg << "\" viewport = \"0 0 " << (float)_image_size.x << " " << (float)_image_size.y << "\" >";
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

		void pixel_scan::compress_shapes()
		{
			for (auto& shape : _shapes)
			{
				shape.compress_chunks();
			}
		}
	}
}