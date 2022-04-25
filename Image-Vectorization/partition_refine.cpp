#include "partition_refine.h"
#include "lsd_interface.h"
#include "regularization_angles.h"
#include "regularization_angles_ms.h"
#include "regularization_ordinates.h"
#include "regularization_ordinates_ms.h"
#include "propagation.h"
#include "refine_partition.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


Partition_Refine::Partition_Refine()
{
}


Partition_Refine::~Partition_Refine()
{
}


void Partition_Refine::extract_segments(Kinetic_Model *model)
{
	model->reinit();
	LSD::find_segments(model);
}


void Partition_Refine::generate(Kinetic_Model *model)
{
    clock_t begin_step;
    model->reinit();

    // Step 1. Runs LSD
    LSD::find_segments(model);
    emit lsd_done();

    if (model->params->rega_regp_enabled) {
        // Step 2. Regularizes segments' angles
        Regularization_Angles* m_rega = new Regularization_Angles_Mean_Shift();
        m_rega->regularize(model);
        delete m_rega;

        // Step 3. Regularizes segments' ordinates
		Regularization_Ordinates* m_regp = new Regularization_Ordinates_Mean_Shift();
		m_regp->regularize(model);
		delete m_regp;
        emit regularization_done();
    }
    model->set_angle_hist_index();

    // Step 4. Propagation
    begin_step = clock();
    Propagation::propagate_image_domain(model);
    emit propagation_done();

	Refine_Partition::make_layer(model);
    model->elapsed_time_building_graph = double(clock() - begin_step) / CLOCKS_PER_SEC;
}

void Partition_Refine::evolve(Kinetic_Model* model)
{
	clock_t begin_step;

	begin_step = clock();

	// Step 5. Merge and split
	Refine_Partition::grouping(model);
	emit evolution_done();

	model->elapsed_time_grouping = double(clock() - begin_step) / CLOCKS_PER_SEC;
	
}
