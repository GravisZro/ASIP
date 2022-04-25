#ifndef PARTITION_H
#define PARTITION_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include "kinetic_model.h"

class Partition_Refine : public QObject
{
    Q_OBJECT

public:
    Partition_Refine();

    ~Partition_Refine();

	void extract_segments(Kinetic_Model* model);

    void generate(Kinetic_Model* model);

	void evolve(Kinetic_Model* model);

signals:
    void lsd_done();

    void regularization_done();

    void propagation_done();

	void preprocessing_done();

	void evolution_done();
};

#endif // PARTITION_H
