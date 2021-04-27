#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>

#include "entrypoint.h"
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

const char *format1_p = "png";
const char *format2_p = "jpeg";

std::vector<char> emergency_memory = std::vector<char>( 16384ull, '0' );

std::unique_ptr<IShapeFinder> shape_finder = std::make_unique<bobsweeper>();

int execute_program(char* input_file_p, int chunk_size, float threshold, char* output_file_p) {
	try
	{
		if (emergency_memory.empty())
			emergency_memory = std::vector<char>(16384ull, '0');

		Image img = convert_png_to_image(input_file_p);

		vectorize_options options = {
			input_file_p,
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


int entrypoint(int argc, char* argv[]) {
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