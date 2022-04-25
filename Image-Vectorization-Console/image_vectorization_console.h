#pragma once
#include "kinetic_model.h"


namespace Partition_Refine 
{
	void process(Kinetic_Model* model);

	void process_single_image(Kinetic_Model* model);

	void process_directory(Kinetic_Model* model);

	void generate(Kinetic_Model* model);

	void evolve(Kinetic_Model* model);

	void save(Kinetic_Model* model, std::string & location, bool final_model = TRUE);

}