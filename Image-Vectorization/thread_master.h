#ifndef THREAD_MASTER_H
#define THREAD_MASTER_H

#include <QThread>
#include "partition_refine.h"
#include "kinetic_model.h"


class Thread_Master : public QThread
{
    Q_OBJECT
public:
    Thread_Master(QObject *parent = 0);

    ~Thread_Master();

	void extract_segments_async(Partition_Refine* method, Kinetic_Model* model);

    void generate_kinetic_partition_async(Partition_Refine* method, Kinetic_Model* model);

	void merge_and_split_async(Partition_Refine* method, Kinetic_Model* model);

signals:
	void segments_extraction_requested(Partition_Refine* method, Kinetic_Model* model);

	void segments_extraction_done(Kinetic_Model* model);

    void kinetic_partition_requested(Partition_Refine* method, Kinetic_Model* model);

    void kinetic_partition_done(Kinetic_Model* model);

	void merge_and_split_async_requested(Partition_Refine* method, Kinetic_Model* model);

	void evolution_done(Kinetic_Model* model);

protected:
    void run();

private:
    bool m_ready;
};

#endif // THREAD_MASTER_H
