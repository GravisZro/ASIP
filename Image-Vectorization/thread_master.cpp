#include "thread_master.h"
#include "thread_worker.h"


Thread_Master::Thread_Master(QObject* parent) :
    QThread (parent),
    m_ready (false)
{
    start();
    while (!m_ready) msleep(50);
}


Thread_Master::~Thread_Master()
{
    quit();
#if QT_VERSION >= QT_VERSION_CHECK(5,2,0)
    requestInterruption();
#endif
    wait();
}


void Thread_Master::extract_segments_async(Partition_Refine* method, Kinetic_Model* model)
{
	emit segments_extraction_requested(method, model);
}


void Thread_Master::generate_kinetic_partition_async(Partition_Refine *method, Kinetic_Model *model)
{
    emit kinetic_partition_requested(method, model);
}

void Thread_Master::merge_and_split_async(Partition_Refine* method, Kinetic_Model* model)
{
	emit merge_and_split_async_requested(method, model);
}

void Thread_Master::run()
{
    Thread_Worker thread_worker;
    qRegisterMetaType<Kinetic_Model>("Kinetic_Model");

    connect(this, SIGNAL(kinetic_partition_requested(Partition_Refine*, Kinetic_Model*)), &thread_worker, SLOT(do_kinetic_partition(Partition_Refine*, Kinetic_Model*)));
    connect(&thread_worker, SIGNAL(kinetic_partition_done(Kinetic_Model*)), this, SIGNAL(kinetic_partition_done(Kinetic_Model*)));

	connect(this, SIGNAL(merge_and_split_async_requested(Partition_Refine*, Kinetic_Model*)), &thread_worker, SLOT(do_evolution(Partition_Refine*, Kinetic_Model*)));
	connect(&thread_worker, SIGNAL(evolution_done(Kinetic_Model*)), this, SIGNAL(evolution_done(Kinetic_Model*)));

    connect(this, SIGNAL(segments_extraction_requested(Partition_Refine*, Kinetic_Model*)), &thread_worker, SLOT(do_segments_extraction(Partition_Refine*, Kinetic_Model*)));
    connect(&thread_worker, SIGNAL(segments_extracted(Kinetic_Model*)), this, SIGNAL(segments_extraction_done(Kinetic_Model*)));

    m_ready = true;
    exec();
}
