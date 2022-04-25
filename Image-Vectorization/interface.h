#pragma once

#include <QtWidgets/QMainWindow>

#include "myqgraphicsview.h"
#include "thread_master.h"
#include "kinetic_model.h"
#include "partition_refine.h"
#include "ui_interface.h"


class Interface : public QMainWindow
{
	Q_OBJECT

public:
	Interface(QWidget *parent = Q_NULLPTR);

	~Interface();

private:
	void read_parameters();

	void connect_widgets();

	void update_label_cells_count();

	void update_evolution_products();

	void load_partitioning_products();

	void unload_partitioning_products();

	void load_matrix_gray_as_colored_qimage(Matrix<uchar> &I, QImage & Q);

	void load_matrix_rgb_as_qimage(Matrix<uchar> & I, QImage & Q);

	void load_matrix_argb_as_qimage(Matrix<uchar> & I, QImage & Q);

	void load_qpixmap_as_qgraphicspixmapitem(QPixmap & P, QGraphicsPixmapItem* & L, QCheckBox *C, double z_value, double alpha_channel);

	void load_qimage_as_qgraphicspixmapitem(QImage & Q, QGraphicsPixmapItem* & L, QCheckBox *C, double z_value, double alpha_channel);

	void load_qgraphics_line_items(list<LineItem *> & source, list<QGraphicsLineItem *> &dest, QCheckBox *checkbox, double z_value);

	void reload_qgraphics_line_items_when_line_width_changes(list<QGraphicsLineItem *> & dest);

	template<typename T>
	void unload_qgraphics_line_items(list<T *> & container) {
		for (typename list<T *>::iterator it_l = container.begin(); it_l != container.end(); it_l++) {
			scene.removeItem(*it_l);
			delete (*it_l);
		}
		container.clear();
	}

	template<typename T>
	void show_qgraphics_line_items(list<T *> & container) {
		for (typename list<T *>::iterator it_l = container.begin(); it_l != container.end(); it_l++) {
			(*it_l)->show();
		}
	}

	template<typename T>
	void hide_qgraphics_line_items(list<T *> & container) {
		for (typename list<T *>::iterator it_l = container.begin(); it_l != container.end(); it_l++) {
			(*it_l)->hide();
		}
	}

	void load_highlighted_facets();

	void unload_highlighted_facets();

	void load_colored_facets();

	void unload_colored_facets();


private slots:
	void open();
	void open_label();
	void open_probability();
	void save();

	void do_lsd();
	void handle_lsd_done();

	void handle_lsd_done_as_intermediate_step();
	void handle_regularization_done_as_intermediate_step();
	void handle_propagation_done_as_intermediate_step();
	void handle_preprocessing_done_as_intermediate_step();
	void handle_generation_done();
	void handle_evolution_done();

	void set_line_width(qreal width);
	void set_alpha_channels();

	void set_iterations();
	void set_alignment_beta();
	void set_prior_lambda();
	void set_split_params();
	void do_evolution();
	void reset_graph();

	void set_visibility_layer_background();
	void set_visibility_layer_input_prediction();
	void set_visibility_layer_split_segments();
	void set_visibility_layer_partition();
	void set_visibility_layer_next_operation();
	void set_visibility_layer_colored_facets();

	void set_semantic_type(Semantic_Type type);

private:
	Thread_Master thread_master;
	bool thread_busy;
	bool use_default_gradient = FALSE;

	Kinetic_Model* model;
	Partition_Refine* kinetic_partition;

	clock_t start_time;
	clock_t end_time;

	QString path_background;
	QString path_input_prediction;

	QPixmap qpixmap_background;
	QPixmap qpixmap_input_prediction;
	QImage qimage_colored_facets;

	QGraphicsScene scene;
	QGraphicsPixmapItem* layer_background;
	QGraphicsPixmapItem* layer_input_prediction;
	list<QGraphicsLineItem *> layer_split_segments;
	list<QGraphicsLineItem *> layer_regularized;
	list<QGraphicsLineItem *> layer_partition;
	list<QGraphicsLineItem *> layer_next_operation;
	QGraphicsPixmapItem* layer_highlighted_facets;
	QGraphicsPixmapItem* layer_colored_facets;


	Semantic_Type semantic_type; // 0 - probability map,  1 - label map,  2 - none

	qreal line_width;

	MyQGraphicsView* zoomable_qgraphicsview;

private:
	Ui::Polygon_InterfaceClass ui;
};
