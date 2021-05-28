#include "entrypoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <optional>

#include "nsvg/usage.h"
#include "nsvg/mapping.h"
#include "nsvg/mapparser.h"
#include "utility/error.h"
#include "utility/logger.h"
#include "imagefile/pngfile.h"
#include "imagefile/svg.h"
#include "nsvg/ishapefinder.h"
#include "nsvg/bobsweep.h"
#include "nsvg/bobsweeperv2.h"
#include "nsvg/dcdfiller.h"
#include "reduce.h"

namespace vectorizer
{
	const std::string version = "v0.69";

	const char* format1_p = "png";
	const char* format2_p = "jpeg";

	namespace
	{
		std::vector<char> emergency_memory = std::vector<char>(16384ull, '0');

		std::unique_ptr<IShapeFinder> shape_finder = std::make_unique<bobsweeper>();
	}

	int execute_program(std::string input_path, int chunk_size, float threshold, std::string output_path) {
		try
		{
			if (emergency_memory.empty())
				emergency_memory = std::vector<char>(16384ull, '0');

			Image img = convert_png_to_image(input_path.c_str());

			vectorize_options options = {
				input_path,
				chunk_size,
				threshold
			};

			chunkmap map{ img, options.chunk_size };

			bobsweeperv2{}.process_image(img, options.shape_colour_threshhold, "your boi.svg");

		}
		catch (std::bad_alloc& ex)
		{
			if (emergency_memory.size())
			{
				emergency_memory.clear();
				std::cerr << "Vectorizer Ran Out of Memory: " << ex.what() << std::endl;
				throw ex;
			}
		}
		catch (std::invalid_argument& e)
		{
			std::cerr << "Vectorizer failed with an invalid argument: " << e.what() << std::endl;
			return BAD_ARGUMENT_ERROR;
		}
		catch (std::exception& e)
		{
			std::cerr << "Vectorizer failed with an unknown exception: " << e.what() << std::endl;
			return UNKNOWN_ERROR;
		}

		return SUCCESS_CODE;
	}

	void print_help_message()
	{
		std::cout << "[Vectorizer " << version << "] help:" << std::endl;
		std::cout << "commands:" << std::endl;
		std::cout << "\t" << "scan [input image path] [threshold] [--output-path/-out/-o] [output data path] [--no-borders/-nb] \n";
		std::cout << "\t\t" << "scans given image ([input image path]) to shape data (with [threshold]) and saves shape data to [output data path] or \"output.sdat\" if not specified \n";
		std::cout << "\t\t" << "--no-borders skips the border computation \n";
		std::cout << "\n";
		std::cout << std::endl;
		std::cout << "\t" << "parse [input data path] [--output-path/-out/-o] [output svg path] \n";
		std::cout << "\t" << "\t" << "parses existing shape data ([input data path]) to an svg file ([output svg path] or \"output.svg\") \n";
		std::cout << "\n";
		std::cout << std::endl;
		std::cout << "\t" << "process [input image path] [threshold] [--output-path/-out/-o] [output svg path] [--shape-path/-s] [shape path]\n";
		std::cout << "\t" << "\t" << "processes input image straight to svg \n";
		std::cout << "\t" << "\t" << "scans given image ([input image path]) to shape data (with [threshold]) and parses into svg file [output svg path] or \"output.svg\" if not specified ([output svg path]) \n";
		std::cout << "\t" << "\t" << "optional [--intermediate-path] [shape path] saves the shape data to [shape path] \n";
		std::cout << "\n";
		std::cout << std::endl;
		std::cout << "\t" << "write-borders [input data path] [--output-path]/-out/-o] [output png path]\n";
		std::cout << "\t" << "write-borders --from-image/-i [input image path] [--output-path/-out/-o] [output png path] [--shape-path/-s] [shape path]\n";
		std::cout << "\t" << "\t" << "usage 1. loads existing data from disk ([input data path]) and writes shape borders to [output png path] or \"borders.png\"e \n";
		std::cout << "\t" << "\t" << "usage 2. scans a given image from disk ([input image path]) and writes shape borders to [output png path] or \"borders.png\" \n";
		std::cout << "\t" << "\t" << "usage 2. --shape-path will write shape data to [shape path]\n";
		std::cout << "\n";
		std::cout << std::endl;
		std::cout << "\t" << "write-shapes [input data path] [output png path]\n";
		std::cout << "\t" << "write-shapes --from-image/-i [input image path] [output png path] [--shape-path/-s] [shape path]\n";
		std::cout << "\t" << "\t" << "usage 1. loads existing data from disk ([input data path]) and writes shapes to [output png path] \n";
		std::cout << "\t" << "\t" << "usage 2. scans a given image from disk ([input image path]) and writes shapes to [output png path] \n";
		std::cout << "\t" << "\t" << "usage 2. --shape-path will write shape data to [shape path]\n";
		std::cout << "\n";
		std::cout << std::endl;
		std::cout << "\t" << "write-shape [shape number] [input data path] [output png path]\n";
		std::cout << "\t" << "\t" << "writes the [shape number]-th shape in the given existing shape data to output png path \n";
		std::cout << "\n";
		std::cout << std::endl;
		std::cout << "\t" << "reduce [input image path] [neighbour reach] [threshold] [output png path]\n";
		std::cout << "\t" << "\t" << "reduces given image (input image path) with [neighbour reach] depth and [threshold] threshold to output png path\n";
		std::cout << "\n";
		std::cout << std::endl;
	}

	int command_normal(const std::vector<std::string>& args);
	int command_scan(const std::vector<std::string>& args);
	int command_parse(const std::vector<std::string>& args);
	int command_process(const std::vector<std::string>& args);
	int command_write_borders(const std::vector<std::string>& args);
	int command_write_border(const std::vector<std::string>& args);
	int command_write_shapes(const std::vector<std::string>& args);
	int command_write_shape(const std::vector<std::string>& args);
	int command_reduce(const std::vector<std::string>& args);

	std::optional<std::string> get_string_after(const std::vector<std::string>& args, const std::vector<std::string>& values)
	{
		for (int i = 0; i < values.size(); ++i)
		{
			auto it = std::find(args.begin(), args.end(), values[i]);
			if (it != args.end() && it + 1 != args.end())
				return *(it + 1);
		}

		return {};
	}

	bool args_contain(const std::vector<std::string>& args, std::vector<std::string> to_find)
	{
		for (const auto& s : to_find)
		{
			if (std::find(args.begin(), args.end(), s) != args.end())
				return true;
		}
		return false;
	}


	int entrypoint(int argc, char* argv[]) 
	{

		std::vector<std::string> args{ argv + 1, argv + argc };

		if (args.size() < 2)
		{
			print_help_message();
			return 0;
		}

		if (args.size() > 1 && args[0].find('.') != std::string::npos)
		{
			return command_normal(args);
		}

		std::string command = args[0];
		args.erase(args.begin());


		try
		{
			if (command == "help" || command == "--help")
			{
				print_help_message();
			}
			else if (command == "scan")
			{
				return command_scan(args);
			}
			else if (command == "parse")
			{
				return command_parse(args);
			}
			else if (command == "process")
			{
				return command_process(args);
			}
			else if (command == "write-borders" || command == "wbs")
			{
				return command_write_borders(args);
			}
			else if (command == "write-border" || command == "wb")
			{
				return command_write_border(args);
			}
			else if (command == "write-shapes" || command == "wss")
			{
				return command_write_shapes(args);
			}
			else if (command == "write-shape" || command == "ws")
			{
				return command_write_shape(args);
			}
			else if (command == "reduce" || "r")
			{
				return command_reduce(args);
			}
			else
			{
				std::cerr << "Unknown command '" << command << "'" << std::endl;
			}
		}
		catch (std::bad_alloc& ex)
		{
			if (emergency_memory.size())
			{
				emergency_memory.clear();
				std::cerr << "Vectorizer Ran Out of Memory: " << ex.what() << std::endl;
				throw ex;
			}
		}
		catch (std::invalid_argument& e)
		{
			std::cerr << "Vectorizer failed with an invalid argument: " << e.what() << std::endl;
			return BAD_ARGUMENT_ERROR;
		}
		catch (std::exception& e)
		{
			std::cerr << "Vectorizer failed with an unknown exception: " << e.what() << std::endl;
			return UNKNOWN_ERROR;
		}


		printf("entrypoint with: ");
		for (int i = 1; i < argc; ++i)
		{
			printf("%s, ", argv[i]);
		}
		printf("\n");

		if (argc <= 1)
		{
			printf("error: No Arguments given");
			return SUCCESS_CODE;
		}

		char* firstargument_p = argv[1];

		// Grab input file path
		char* input_file_p = firstargument_p;

		// Grag output file path
		char* output_file_p;

		// If no output path given use default one
		if (argc > 2)
			output_file_p = "output.svg";
		else
			output_file_p = argv[2];

		int chunk_size = 0;
		if (argc > 3)
			chunk_size = atoi(argv[3]);

		if (chunk_size < 1)
			chunk_size = 4;

		float threshold = 0.f;
		if (argc > 4)
			threshold = (float)atof(argv[4]);

		printf("atof-ed threshold=%f", threshold);

		if (threshold < 0.f)
			threshold = 0.f;

		// Halt execution if either path is bad
		if (input_file_p == NULL || output_file_p == NULL)
		{
			printf("Empty input or output file");
			return SUCCESS_CODE;
		}

		printf("Vectorizing with input: '%s' output: '%s' chunk size: '%d' threshold: '%f' \n", input_file_p, output_file_p, chunk_size, threshold);

		return execute_program(input_file_p, chunk_size, threshold, output_file_p);
	}

	int set_algorithm(int algo)
	{
		switch (algo)
		{
		case 0:
			shape_finder = std::make_unique<dcdfiller>();
			break;
		case 1:
			shape_finder = std::make_unique<bobsweeper>();
			break;
		default:
			return BAD_ARGUMENT_ERROR;
		}
		return 0;
	}

	int command_normal(const std::vector<std::string>& args)
	{
		// Normal execution
		std::string input_path = args[1];
		std::string output_path = args[2];

		float threshold = 40.f;
		if (args.size() > 3)
		{
			try
			{
				threshold = std::stof(args[3]);
			}
			catch (std::invalid_argument& e) {}
		}
		int chunk_size = 4;
		if (args.size() > 4)
		{
			try
			{
				chunk_size = std::stoul(args[4]);
			}
			catch (std::invalid_argument& e) {}
		}

		return execute_program(input_path, chunk_size, threshold, output_path);
	}

	int command_scan(const std::vector<std::string>& args)
	{
		if (args.size() <= 2)
		{
			std::cout << "scan requires at least an input path and threshold" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or("output.sdat");
			std::cout << "scanning '" << input_path << "' to '" << output_path << "'..." << std::endl;

			bool skip_borders = args_contain(args, { "--no-borders", "-nb" });

			float threshold = std::stof(args[1]);

			Image input_img = Image{ input_path };

			scan::pixel_scan scan{ input_img };

			scan.scan_for_shapes(threshold);
			if (!skip_borders)
				scan.calculate_borders();

			std::ofstream out_file = std::ofstream{ output_path };
			scan.compress_shapes();
			scan.Serialize(out_file);

			std::cout << "Successfully scanned." << std::endl;
			std::cout << "Written file to '" << output_path << "'." << std::endl;
			std::cout << "Found " << scan.shapes().size() << " shapes." << std::endl;
		}

		return 0;
	}

	int command_parse(const std::vector<std::string>& args)
	{
		if (args.size() <= 1)
		{
			std::cout << "parse needs at least an input path!" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or("output.sdat");

			std::cout << "parsing '" << input_path << "' to '" << output_path << "'..." << std::endl;

			std::ifstream input_stream{ input_path };
			scan::pixel_scan scan = scan::pixel_scan{ input_stream };

			scan.to_svg(output_path);

			std::cout << "Successful parse" << std::endl;
		}

		return 0;
	}

	int command_process(const std::vector<std::string>& args)
	{
		if (args.size() <= 1)
		{
			std::cout << "process needs at least an input path and threshold!" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or("output.svg");
			float threshold = std::stof(args[1]);

			auto shape_path_opt = get_string_after(args, { "--shape-path", "-sb" });

			std::cout << "parsing '" << input_path << "' to '" << output_path << "'..." << std::endl;

			Image input_img{ input_path };

			scan::pixel_scan scan = scan::pixel_scan{ input_img };

			scan.scan_for_shapes(threshold);

			scan.calculate_borders();

			if (shape_path_opt)
			{
				scan.compress_shapes();
				scan.Serialize(std::ofstream{ *shape_path_opt });
			}

			scan.to_svg(output_path);

			std::cout << "Successfully processed \'" << input_path << "\' to \'" << output_path << "\' with " << scan.shapes().size() << " shapes " << std::endl;
		}

		return 0;
	}

	int command_write_borders(const std::vector<std::string>& args)
	{
		if (args.size() < 1)
		{
			std::cout << "write-borders needs an input path!" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or("borders.png");



			std::ifstream input_file{ input_path };
			scan::pixel_scan scan{ input_file };

			scan.calculate_borders();

			scan.borders_to_png(output_path);

			std::cout << "Successfully writen " << scan.shapes().size() << " borders from \'" << input_path << "\' to \'" << output_path << "\'" << std::endl;
		}

		return 0;
	}

	int command_write_border(const std::vector<std::string>& args)
	{
		if (args.size() < 2)
		{
			std::cout << "write-border needs an input path and a number!" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			size_t shape_index = std::stoul(args[1]);

			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or(std::string("border ") + std::to_string(shape_index) + ".png");

			std::ifstream input_file{ input_path };
			scan::pixel_scan scan{ input_file };

			if (shape_index >= scan.shapes().size())
			{
				LOG_ERR("Shape %u is too high! Scan only has %u shapes", shape_index, scan.shapes().size());
				return -1;
			}

			scan.calculate_borders();

			scan.border_to_png(shape_index, output_path);

			std::cout << "Successfully writen shape " << shape_index << "'s borders from \'" << input_path << "\' to \'" << output_path << "\'" << std::endl;
		}

		return 0;
	}

	int command_write_shapes(const std::vector<std::string>& args)
	{
		if (args.size() < 1)
		{
			std::cout << "write-shapes needs an input path!" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or("shapes.png");



			std::ifstream input_file{ input_path };
			scan::pixel_scan scan{ input_file };

			scan.shapes_to_png(output_path);

			std::cout << "Successfully writen " << scan.shapes().size() << " borders from \'" << input_path << "\' to \'" << output_path << "\'" << std::endl;
		}

		return 0;
	}

	int command_write_shape(const std::vector<std::string>& args)
	{
		if (args.size() < 2)
		{
			std::cout << "write-shapes needs an input path and a number!" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			size_t shape_index = std::stoul(args[1]);

			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or(std::string("shape ") + std::to_string(shape_index) + ".png");

			std::ifstream input_file{ input_path };
			scan::pixel_scan scan{ input_file };

			if (scan.shapes().size() <= shape_index)
			{
				LOG_ERR("Shape %u is too high! Data only has %u shapes!", scan.shapes().size());
				return -1;
			}

			scan.shape_to_png(shape_index, output_path);

			std::cout << "Successfully writen shape " << shape_index << " from \'" << input_path << "\' to \'" << output_path << "\'" << std::endl;
		}

		return 0;
	}

	int command_reduce(const std::vector<std::string>& args)
	{
		if (args.size() <= 0)
		{
			std::cout << "reduce needs at least a path, neighbour reach and threshold!" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			size_t reach = std::stoul(args[1]);
			float threshold = std::stof(args[2]);
			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or("reduced.png");

			reducer reduction{ reach, threshold };

			Image to_reduce = Image(input_path);

			Image reduced = reduction.reduce_image(to_reduce);

			reduced.to_png(output_path.c_str());


			std::cout << "Successfully reduced \'" << input_path << "\' to \'" << output_path << "\' with reach: " << reach << ", and threshold: " << threshold << std::endl;
		}

		return 0;
	}

	int command_template(const std::vector<std::string>& args)
	{
		if (args.size() <= 0)
		{
			std::cout << "[command] needs at least MINIMUM_ARGS!" << std::endl;
			return -1;
		}
		else
		{
			std::string input_path = args[0];
			std::string output_path = get_string_after(args, { "--output-path", "-out", "-o" }).value_or("output.svg");
			


			std::cout << "Successfully [command] \'" << input_path << "\' to \'" << output_path << "\' with " << std::endl;
		}

		return 0;
	}
}