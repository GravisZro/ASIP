#include "image_vectorization_console.h"
#include "lsd_interface.h"
#include "regularization_angles.h"
#include "regularization_angles_ms.h"
#include "regularization_ordinates.h"
#include "regularization_ordinates_ms.h"
#include "propagation.h"
#include "refine_partition.h"
#include "trace.h"
#include <ctime>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>


using namespace boost::filesystem;
using boost::algorithm::to_lower;


void Partition_Refine::process(Kinetic_Model* model)
{
	if (!model->params->path_input_image.empty()) {

		path input(model->params->path_input_image);
		path input_pred(model->params->path_input_prediction);

		if (exists(input) && is_regular_file(input) && exists(input_pred) && is_regular_file(input_pred)) { 
			process_single_image(model);
		} else {
			std::cerr << "Error : a specified file is not a regular file, or doesn't exist." << std::endl;
		}

	} else if (!model->params->path_input_directory.empty()) {
		
		// A whole directory as input
		path input(model->params->path_input_directory);
		path input_pred(model->params->path_input_prediction_directory);
		if (exists(input) && is_directory(input) && exists(input_pred) && is_directory(input_pred)) {
			process_directory(model);
		} else {
			std::cerr << "Error : a specified path is not a regular folder, or doesn't exist." << std::endl;
		}
	}
}


void Partition_Refine::generate(Kinetic_Model* model)
{
	clock_t begin_step;
    model->reinit();

    // Step 1. Runs LSD
    LSD::find_segments(model);

    if (model->params->rega_regp_enabled && model->segments.size()>1) {

        // Step 2. Regularizes segments' angles
        Regularization_Angles* m_rega = new Regularization_Angles_Mean_Shift();
        m_rega->regularize(model);
        delete m_rega;

        // Step 3. Regularizes segments' ordinates
		Regularization_Ordinates* m_regp = new Regularization_Ordinates_Mean_Shift();
		m_regp->regularize(model);
		delete m_regp;
    }
	model->set_angle_hist_index();

    // Step 4. Propagation
    begin_step = clock();
    Propagation::propagate_image_domain(model);

    model->elapsed_time_building_graph = double(clock() - begin_step) / CLOCKS_PER_SEC;
}


void Partition_Refine::evolve(Kinetic_Model* model)
{
	clock_t begin_step;

	begin_step = clock();

	// Step 5. Merge and split
	Refine_Partition::grouping(model);

	model->elapsed_time_grouping = double(clock() - begin_step) / CLOCKS_PER_SEC;
}


void Partition_Refine::process_single_image(Kinetic_Model* model)
{
	// First of all, we need to create an arborescence at the specified location
	model->set_time_string();
	model->set_basename();

	std::string final_location = model->params->path_output_directory;
	create_directories(path(final_location));

	generate(model);

	evolve(model);
	save(model, final_location, TRUE);
}


void Partition_Refine::save(Kinetic_Model* model, std::string & location, bool final_model)
{
	if (final_model) {
		// Now we can save the desired outputs
		model->graph->save_svg(location, model->basename, false);
		//model->graph->save_faces(location, model->basename, false);
		model->graph->save_graph_definition(location, model->basename);
		model->graph->save_labels(location, model->basename, model->label_id_to_class);
	}
	else {
		// Now we can save the desired outputs
		std::string basename_init = model->basename + "_0";
		//model->initial_graph->save_faces(location, basename_init, false);
		model->initial_graph->save_graph_definition(location, basename_init);
		model->initial_graph->save_svg(location, model->basename, false);
		//model->initial_graph->save_labels(location, basename_init, model->label_id_to_class);
	}
}


void Partition_Refine::process_directory(Kinetic_Model* model)
{
	// First of all, we create an arborescence to save the results produced by the algorithm
	model->set_time_string();
	
	std::string final_location = model->params->path_output_directory;
	std::string final_location_init = model->params->path_output_directory + "\\initial";
	create_directories(path(final_location));
	create_directories(path(final_location_init));

	// List files in input directory
	path input_directory(model->params->path_input_directory);
	path input_prediction_directory(model->params->path_input_prediction_directory);
	vector<path> input_files, input_prediction_files;
	copy(directory_iterator(input_directory), directory_iterator(), std::back_inserter(input_files));
	bool prediction_directory_exist = false;
	if (!model->params->path_input_prediction_directory.empty()) {
		if (exists(input_prediction_directory) && is_directory(input_prediction_directory)) {
			prediction_directory_exist = true;
		}
	}
	clock_t elapsed_time = 0;


	// For every file found
	for (int i = 0 ; i < input_files.size() ; i++) {
	
		std::string ext = extension(input_files[i]);
		to_lower(ext);
		if (ext == ".tif" || ext == ".tiff" || ext == ".jpg" || ext == ".jpeg" || ext == ".png") {

			// Sets the path for the current input image
			model->params->path_input_image = input_files[i].string();
			model->params->path_input_prediction.clear();
			model->params->path_input_partition.clear();
			if (prediction_directory_exist) { // segmentation map images have the same order as the input images
				std::string prediction_filename = input_files[i].stem().string() + ext;
				path input_prediction = input_prediction_directory / prediction_filename;
				model->params->path_input_prediction = input_prediction.string();
				// Skip the image if the probability map does not exist in the provided directory
				if (!exists(input_prediction)) continue;
			} else {
				std::cout << "Could not find the corresponding semantic image for " << input_files[i].stem().string() << "." << std::endl;
			}


			trace(model->params->verbose_level, 0, "Processes : " + input_files[i].filename().string());

			// Processes it and saves the outputs at the specified location
			clock_t begin = clock();
			generate(model);

			evolve(model);
			clock_t end = clock();
			elapsed_time += (end - begin);
			std::cout << "Elapsed time : " << double(elapsed_time) / CLOCKS_PER_SEC << " s." << std::endl;

			save(model, final_location, TRUE);

			int n_vertices = 0;
			Vertex* p_v = model->graph->vertices_head;
			while (p_v != nullptr) {
				for (int j = 0; j < p_v->directions.size(); ++j) {
					Face* f1 = p_v->directions[j].second->f;
					Face* f2 = p_v->directions[j].second->opposite()->f;
					if (f1 != nullptr && f2 != nullptr && f1->label != f2->label && f1->label != INVALID_LABEL && f2->label != INVALID_LABEL) {
						n_vertices++; break;
					}
				}
				// Iterates
				p_v = p_v->v_next;
			}

		}
	}
	
}