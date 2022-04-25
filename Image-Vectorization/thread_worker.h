#ifndef THREAD_WORKER_H
#define THREAD_WORKER_H

#include <QObject>
#include "kinetic_model.h"
#include "partition_refine.h"


class Thread_Worker : public QObject
{
    Q_OBJECT
signals:
    void kinetic_partition_done(Kinetic_Model* model);

	void evolution_done(Kinetic_Model* model);

	void segments_extracted(Kinetic_Model* model);

public slots:
    void do_kinetic_partition(Partition_Refine* method, Kinetic_Model* model);

	void do_segments_extraction(Partition_Refine* method, Kinetic_Model* model);

	void do_evolution(Partition_Refine* method, Kinetic_Model* model);
};

#endif // THREAD_WORKER_H
