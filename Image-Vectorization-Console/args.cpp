#include "args.h"
#include <iostream>
#include <cstring>



bool Args::parse(int argc, char *argv[], Parameters* params)
{
	if (argc == 1) {
		help(); return false;
	}
	int r = 1;
	while (r < argc) {
		if (!strcmp(argv[r], "--input") && r + 1 < argc) {
			params->path_input_image = argv[r + 1];
			r += 2;
		} else if (!strcmp(argv[r], "--input-label") && r + 1 < argc) {
			params->path_input_prediction = argv[r + 1];
			params->semantic_type = LABEL;
			r += 2;
		} else if (!strcmp(argv[r], "--input-probability") && r + 1 < argc) {
			params->path_input_prediction = argv[r + 1];
			params->semantic_type = PROBABILITY;
			r += 2;
		} else if (!strcmp(argv[r], "--input-directory") && r + 1 < argc) {
			params->path_input_directory = argv[r + 1];
			r += 2;
		} else if (!strcmp(argv[r], "--input-label-directory") && r + 1 < argc) {
			params->path_input_prediction_directory = argv[r + 1];
			params->semantic_type = LABEL;
			r += 2;
		} else if (!strcmp(argv[r], "--input-probability-directory") && r + 1 < argc) {
			params->path_input_prediction_directory = argv[r + 1];
			params->semantic_type = PROBABILITY;
			r += 2;
		} else if (!strcmp(argv[r], "--output-directory") && r + 1 < argc) {
			params->path_output_directory = argv[r + 1];
			r += 2;
		} else if (!strcmp(argv[r], "--suffix") && r + 1 < argc) {
			params->suffix = argv[r + 1];
			r += 2;
		} else if (!strcmp(argv[r], "--iterations") && r + 1 < argc) {
			params->max_iters = atoi(argv[r + 1]);
			r += 2;
		} else if (!strcmp(argv[r], "--fidelity-beta")) {
			params->fidelity_beta = atof(argv[r + 1]);
			r += 2;
		} else if (!strcmp(argv[r], "--prior-lambda")) {
			params->prior_lambda = atof(argv[r + 1]);
			r += 2;
		} else if (!strcmp(argv[r], "--verbose-level") && r + 1 < argc) {
			params->verbose_level = atoi(argv[r + 1]);
			r += 2;
		} else if (!strcmp(argv[r], "--help") || !strcmp(argv[r], "-h")) {
			help(); exit(0);
		} else {
			std::cout << "What's this ? " << argv[r] << std::endl;
			help(); return false;
		}
	}
	return true;
}


void Args::help()
{
	std::cout << "Image-Vectorization-Console.exe. Description of the options. " << std::endl;
	std::cout << std::endl;
	std::cout << "Common options" << std::endl;
	std::cout << "--iterations                     Maximum number of iterations" << std::endl;
	std::cout << "--fidelity-beta                  The weight of edge alignment (default: 0.0002)" << std::endl;
	std::cout << "--prior-lambda                   The weight on the complexity term (default: 0.000001)" << std::endl;
	std::cout << "--output-directory               Specify an output directory" << std::endl;
	std::cout << "--suffix               		   Suffix to be added to output files" << std::endl;
	std::cout << "--verbose-level                  Specify the verbose level for console display (default: 3)" << std::endl;
	std::cout << std::endl;
	std::cout << "Single input options" << std::endl;
	std::cout << "--input                          Specify an input image (TIF, JPEG, PNG)" << std::endl;
	std::cout << "--input-probability              Specify an input probability map (TIF, one channel per class)" << std::endl;
	std::cout << "--input-label                    Specify an input pixel-wise semantic labels (TIF, PNG)" << std::endl;
	std::cout << std::endl;
	std::cout << "Multiple input options" << std::endl;
	std::cout << "--input-directory                Specify a directory containing input images (TIF, JPEG, PNG)" << std::endl;
	std::cout << "--input-probability-directory    Specify a directory containing input probability maps (TIF, one channel per class), the filenames should be the same as the images" << std::endl;
	std::cout << "--input-label-directory          Specify a directory containing input pixel-wise semantic labels (TIF, PNG), the filenames should be the same as the images" << std::endl;
	
}