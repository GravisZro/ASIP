#include "thread_worker.h"


void Thread_Worker::do_kinetic_partition(Partition_Refine *method, Kinetic_Model *model)
{
    method->generate(model);
    emit kinetic_partition_done(model);
}


void Thread_Worker::do_segments_extraction(Partition_Refine* method, Kinetic_Model* model)
{
	method->extract_segments(model);
	emit segments_extracted(model);
}

void Thread_Worker::do_evolution(Partition_Refine* method, Kinetic_Model* model)
{
	method->evolve(model);
	emit evolution_done(model);
}