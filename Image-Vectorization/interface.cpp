#include "interface.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <iostream>
#include <fstream>

#ifndef MINSK
#define MINSK 0
#endif
using std::min;
using std::max;

Interface::Interface(QWidget *parent)
	: QMainWindow(parent),
	thread_busy(false),
	line_width(1)
{
	ui.setupUi(this);
	GDALAllRegister();

	ui.graphicsView->setScene(&scene);

	layer_background = nullptr;
	layer_input_prediction = nullptr;
	layer_highlighted_facets = nullptr;
	layer_colored_facets = nullptr;
	layer_regularized = list<QGraphicsLineItem *>();
	layer_partition = list<QGraphicsLineItem *>();
	layer_next_operation = list<QGraphicsLineItem *>();
	layer_split_segments = list<QGraphicsLineItem *>();

	semantic_type = NONE;

	model = new Kinetic_Model();
	read_parameters();

	kinetic_partition = new Partition_Refine();

	ui.doubleSpinBox_line_width->setValue(line_width);

	connect_widgets();
}


Interface::~Interface()
{
	delete kinetic_partition;
	delete model;
}


void Interface::read_parameters()
{
	set_iterations();
	set_alignment_beta();
	set_prior_lambda();
	set_split_params();
}

void Interface::load_matrix_gray_as_colored_qimage(Matrix<uchar> &I, QImage & Q)
{
	if (I.empty()) {
		Q = QImage();
	}
	else {
		uint rows = I.rows, cols = I.cols;
		Q = QImage(int(cols), int(rows), QImage::Format_RGB32);
		// prepare palette
		QVector<QRgb> sColorTable(256);
		sColorTable[0] = qRgb(0, 0, 0);
		std::default_random_engine generator;
		std::uniform_int_distribution<int> uniform_dist(100, 255);
		for (int i = 1; i < 256; ++i) {
			int r = uniform_dist(generator);
			int g = uniform_dist(generator);
			int b = uniform_dist(generator);
			sColorTable[i] = qRgb(r, g, b);
		}

		for (uint i = 0; i < rows; i++) {
			QRgb* line = (QRgb *)Q.scanLine(int(i));
			for (uint j = 0; j < cols; j++) {
				int r = int(I(i, j, 0));
				*line = sColorTable[r];
				++line;
			}
		}
		
	}
}

void Interface::load_matrix_rgb_as_qimage(Matrix<uchar> &I, QImage & Q)
{
	if (I.empty()) {
		Q = QImage();
	} else {
		uint rows = I.rows, cols = I.cols;
		Q = QImage(int(cols), int(rows), QImage::Format_RGB32);
		for (uint i = 0; i < rows; i++) {
			QRgb* line = (QRgb *)Q.scanLine(int(i));
			for (uint j = 0; j < cols; j++) {
				uchar r = I(i, j, 0);
				uchar g = I(i, j, 1);
				uchar b = I(i, j, 2);
				*line = qRgb(int(r), int(g), int(b));
				++line;
			}
		}
	}
}


void Interface::load_matrix_argb_as_qimage(Matrix<uchar> &I, QImage & Q)
{
	if (I.empty()) {
		Q = QImage();
	} else {
		uint rows = I.rows, cols = I.cols;
		Q = QImage(int(cols), int(rows), QImage::Format_ARGB32);
		for (uint i = 0; i < rows; i++) {
			QRgb* line = (QRgb *)Q.scanLine(int(i));
			for (uint j = 0; j < cols; j++) {
				uchar r = I(i, j, 0);
				uchar g = I(i, j, 1);
				uchar b = I(i, j, 2);
				uchar a = I(i, j, 3);
				*line = qRgba(int(r), int(g), int(b), int(a));
				++line;
			}
		}
	}
}


void Interface::load_qpixmap_as_qgraphicspixmapitem(QPixmap & P, QGraphicsPixmapItem* & L, QCheckBox *C, double z_value, double alpha_channel)
{
	if (L != NULL) {
		scene.removeItem(L);
		delete L;
	}

	QPixmap output = QPixmap(P.size());
	output.fill(Qt::transparent);

	QPainter painter(&output);
	painter.setOpacity(alpha_channel);
	painter.drawPixmap(0, 0, P);
	painter.end();

	L = new QGraphicsPixmapItem(output);
	L->setZValue(z_value);
	scene.addItem(L);
	L->setVisible(C->isChecked());
}


void Interface::load_qimage_as_qgraphicspixmapitem(QImage & Q, QGraphicsPixmapItem* & L, QCheckBox *C, double z_value, double alpha_channel)
{
	if (L != NULL) {
		scene.removeItem(L);
		delete L;
	}

	QPixmap input = QPixmap::fromImage(Q);
	QPixmap qpixmap_background = QPixmap(input.size());

	qpixmap_background.fill(Qt::transparent);
	QPainter painter(&qpixmap_background);
	painter.setOpacity(alpha_channel);
	painter.drawPixmap(0, 0, input);
	painter.end();

	L = new QGraphicsPixmapItem(qpixmap_background);
	L->setZValue(z_value);
	scene.addItem(L);
	L->setVisible(C->isChecked());
}


void Interface::load_qgraphics_line_items(list<LineItem *> & source, list<QGraphicsLineItem *> & dest, QCheckBox* checkbox, double z_value)
{
	for (list<LineItem *>::iterator it_l = source.begin() ; it_l != source.end() ; it_l++) {
		LineItem* l = (*it_l);

		QPen pen = QPen(QColor(qRgb(l->r, l->g, l->b)));
#if MINSK
		pen.setWidth(z_value);
#else
		pen.setWidthF(line_width);
#endif
		pen.setCapStyle(Qt::RoundCap);

		QGraphicsLineItem* ql = new QGraphicsLineItem(l->x1, l->y1, l->x2, l->y2);
		ql->setPen(pen);
		ql->setVisible(checkbox->isChecked());
		ql->setZValue(z_value);
		dest.push_back(ql);

		scene.addItem(ql);
	}
}


void Interface::reload_qgraphics_line_items_when_line_width_changes(list<QGraphicsLineItem *> & dest)
{
	for (list<QGraphicsLineItem *>::iterator it_l = dest.begin() ; it_l != dest.end() ; it_l++) {
		QGraphicsLineItem* ql = (*it_l);
		QPen pen = ql->pen();
		pen.setWidthF(line_width);
		ql->setPen(pen);
	}
}



void Interface::connect_widgets()
{
	connect(ui.pushButton_open, SIGNAL(released()), this, SLOT(open()));
	connect(ui.pushButton_open_label, SIGNAL(released()), this, SLOT(open_label()));
	connect(ui.pushButton_open_probability, SIGNAL(released()), this, SLOT(open_probability()));
	connect(ui.pushButton_save, SIGNAL(released()), this, SLOT(save()));
	connect(ui.pushButton_merge_and_split, SIGNAL(released()), this, SLOT(do_evolution()));
	connect(ui.pushButton_merge_and_split_reset, SIGNAL(released()), this, SLOT(reset_graph()));

	connect(&thread_master, SIGNAL(segments_extraction_done(Kinetic_Model*)), this, SLOT(handle_lsd_done()));
	connect(&thread_master, SIGNAL(kinetic_partition_done(Kinetic_Model*)), this, SLOT(handle_generation_done()));
	connect(&thread_master, SIGNAL(evolution_done(Kinetic_Model*)), this, SLOT(handle_evolution_done()));

	connect(kinetic_partition, SIGNAL(lsd_done()), this, SLOT(handle_lsd_done_as_intermediate_step()));
	connect(kinetic_partition, SIGNAL(regularization_done()), this, SLOT(handle_regularization_done_as_intermediate_step()));
	connect(kinetic_partition, SIGNAL(propagation_done()), this, SLOT(handle_propagation_done_as_intermediate_step()));
	connect(kinetic_partition, SIGNAL(preprocessing_done()), this, SLOT(handle_preprocessing_done_as_intermediate_step()));

	connect(ui.spinBox_iterations, SIGNAL(editingFinished()), this, SLOT(set_iterations()));
	connect(ui.doubleSpinBox_alignment_beta, SIGNAL(editingFinished()), this, SLOT(set_alignment_beta()));
	connect(ui.doubleSpinBox_prior_lambda, SIGNAL(editingFinished()), this, SLOT(set_prior_lambda()));
	connect(ui.doubleSpinBox_split_length_thresh, SIGNAL(editingFinished()), this, SLOT(set_split_params()));

	connect(ui.checkBox_display_background, SIGNAL(released()), this, SLOT(set_visibility_layer_background()));
	connect(ui.checkBox_display_input_prediction, SIGNAL(released()), this, SLOT(set_visibility_layer_input_prediction()));
	connect(ui.checkBox_display_split_segments, SIGNAL(released()), this, SLOT(set_visibility_layer_split_segments()));
	connect(ui.checkBox_display_partition, SIGNAL(released()), this, SLOT(set_visibility_layer_partition()));
	connect(ui.checkBox_display_partition, SIGNAL(released()), this, SLOT(set_visibility_layer_active_gradients()));
	connect(ui.checkBox_display_next_operation, SIGNAL(released()), this, SLOT(set_visibility_layer_next_operation()));
	connect(ui.checkBox_display_colored_facets, SIGNAL(released()), this, SLOT(set_visibility_layer_colored_facets()));

	connect(ui.doubleSpinBox_prediction_alpha, SIGNAL(valueChanged(double)), this, SLOT(set_alpha_channels()));
	connect(ui.doubleSpinBox_colored_facets_alpha, SIGNAL(valueChanged(double)), this, SLOT(set_alpha_channels()));
	connect(ui.doubleSpinBox_line_width, SIGNAL(valueChanged(double)), this, SLOT(set_line_width(double)));
}



void Interface::open()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Please open an RGB image"), ".", tr("Image files (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)"));
	if (!filename.isNull()) {
		QImage qimage = QImage(filename);
		if (!qimage.isNull()) {
			if (layer_background != nullptr) {
				scene.removeItem(layer_background);
				delete layer_background;
			}

			qpixmap_background = QPixmap::fromImage(qimage);
			layer_background = new QGraphicsPixmapItem(qpixmap_background);
			scene.addItem(layer_background);
			scene.setSceneRect(QRect(0, 0, qimage.width(), qimage.height()));
			ui.graphicsView->centerOn(QPointF(qimage.width() / 2, qimage.height() / 2));
            ui.graphicsView->show();

			path_background = filename;
			model->set_path_input_image(path_background.toStdString());
			ui.checkBox_display_background->setChecked(true);
			unload_partitioning_products();

		} else {
			QMessageBox::warning(this, tr(windowTitle().toStdString().c_str()), tr("Could not load the selected image."));
		}
	}
}

void Interface::open_label()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Please open a grayscale label map"), ".", tr("Image files (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)"));
	if (!filename.isNull()) {
		QImage qimage = QImage(filename);
		if (!qimage.isNull()) {
			if (layer_input_prediction != nullptr) {
				scene.removeItem(layer_input_prediction);
				delete layer_input_prediction;
			}

			if (!qimage.isGrayscale()) QMessageBox::warning(this, tr(windowTitle().toStdString().c_str()), tr("Could not load the selected image. Please choose an 8-bit grayscale image"));

			// color-code for visualization purpose
			// First convert Format_Grayscale8 to Format_Indexed8 to enable color table
			qimage = qimage.convertToFormat(QImage::Format_Indexed8);
			QVector<QRgb> sColorTable(256);
			sColorTable[0] = qRgb(0, 0, 0);
			std::default_random_engine generator;
			std::uniform_int_distribution<int> uniform_dist(100, 255);
			for (int i = 1; i < 256; ++i) {
				int r = uniform_dist(generator);
				int g = uniform_dist(generator);
				int b = uniform_dist(generator);
				sColorTable[i] = qRgb(r, g, b);
			}
			qimage.setColorTable(sColorTable);

			qpixmap_input_prediction = QPixmap::fromImage(qimage);
			load_qpixmap_as_qgraphicspixmapitem(qpixmap_input_prediction, layer_input_prediction, ui.checkBox_display_input_prediction, 1.0, ui.doubleSpinBox_prediction_alpha->value());
			path_input_prediction = filename;
			model->set_path_input_prediction(path_input_prediction.toStdString());
			ui.checkBox_display_input_prediction->setChecked(false);
			set_semantic_type(LABEL);
			unload_partitioning_products();
		}
		else {
			QMessageBox::warning(this, tr(windowTitle().toStdString().c_str()), tr("Could not load the selected image."));
		}
	}
}

void Interface::open_probability()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Please open a probability map"), ".", tr("Image files (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)"));
	if (!filename.isNull()) {
		QImage qimage = QImage(filename);
		if (!qimage.isNull()) {
			if (layer_input_prediction != nullptr) {
				scene.removeItem(layer_input_prediction);
				delete layer_input_prediction;
			}
			if (!qimage.isGrayscale()) QMessageBox::warning(this, tr(windowTitle().toStdString().c_str()), tr("Could not load the selected image. Please choose a grayscale image"));

			qpixmap_input_prediction = QPixmap::fromImage(qimage);
			load_qpixmap_as_qgraphicspixmapitem(qpixmap_input_prediction, layer_input_prediction, ui.checkBox_display_input_prediction, 1.0, ui.doubleSpinBox_prediction_alpha->value());
			path_input_prediction = filename;
			model->set_path_input_prediction(path_input_prediction.toStdString());
			ui.checkBox_display_input_prediction->setChecked(false);
			set_semantic_type(PROBABILITY);
			unload_partitioning_products();
		}
		else {
			QMessageBox::warning(this, tr(windowTitle().toStdString().c_str()), tr("Could not load the selected image."));
		}
	}
}

void Interface::save()
{
#if 1
	if (layer_partition.empty()) return;

	QString directory = QFileDialog::getExistingDirectory(this, tr("Please select a directory where to save the outputs"), ".",
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!directory.isNull()) {

		try {
			QDir subdirectory(directory + QString(separator) + QString::fromStdString(model->basename + "_" + model->time_string));
			subdirectory.mkpath(".");

			// Saves a screenshot of the scene
			scene.setSceneRect(scene.itemsBoundingRect());
			//QImage image(scene.sceneRect().size().toSize(), QImage::Format_ARGB32);
			QImage image(qpixmap_background.size(), QImage::Format_ARGB32);

			QPainter painter(&image);
			scene.render(&painter);
			image.save(subdirectory.path()
				+ QString(separator)
				+ QString::fromStdString(model->basename)
				+ QString("_snapshot.bmp"), "BMP", 100);


			model->graph->save_boundaries(subdirectory.path().toStdString(), model->basename);
			model->graph->save_edges(model->I, subdirectory.path().toStdString(), model->basename, 1.0);
			model->graph->save_svg(subdirectory.path().toStdString(), model->basename, false);
			model->graph->save_graph_definition(subdirectory.path().toStdString(), model->basename);

			// Saves labels
			model->graph->save_labels(subdirectory.path().toStdString(), model->basename, model->label_id_to_class);


			QMessageBox::information(this, tr(windowTitle().toStdString().c_str()), tr("The output files have been successfully saved."));

		}
		catch (const std::exception & e) {
			QMessageBox::warning(this, tr(windowTitle().toStdString().c_str()), tr(e.what()));
		}
	}
#endif
}


void Interface::set_line_width(double width)
{
	line_width = width;
	reload_qgraphics_line_items_when_line_width_changes(layer_split_segments);
	reload_qgraphics_line_items_when_line_width_changes(layer_regularized);
	reload_qgraphics_line_items_when_line_width_changes(layer_partition);
	reload_qgraphics_line_items_when_line_width_changes(layer_next_operation);
}


void Interface::set_alpha_channels()
{
	double input_prediction_alpha = ui.doubleSpinBox_prediction_alpha->value();
	double facets_alpha = ui.doubleSpinBox_colored_facets_alpha->value();
	
	if (ui.checkBox_display_colored_facets->isChecked()) {
		double background_alpha = 1 - facets_alpha;
		ui.doubleSpinBox_background_alpha->setValue(background_alpha);
	}
	else if (ui.checkBox_display_input_prediction->isChecked()) {
		double background_alpha = 1 - input_prediction_alpha;
		ui.doubleSpinBox_background_alpha->setValue(background_alpha);
	}
	else {
		ui.doubleSpinBox_background_alpha->setValue(1);
	}
	

	load_qpixmap_as_qgraphicspixmapitem(qpixmap_background, layer_background, ui.checkBox_display_background, 0.0, ui.doubleSpinBox_background_alpha->value());
	load_qpixmap_as_qgraphicspixmapitem(qpixmap_input_prediction, layer_input_prediction, ui.checkBox_display_input_prediction, 0.5, input_prediction_alpha);
	load_qimage_as_qgraphicspixmapitem(qimage_colored_facets, layer_colored_facets, ui.checkBox_display_colored_facets, 1.5, ui.doubleSpinBox_colored_facets_alpha->value());
}

void Interface::set_iterations()
{
	model->params->max_iters = ui.spinBox_iterations->value();
}

void Interface::set_alignment_beta()
{
	model->params->fidelity_beta = ui.doubleSpinBox_alignment_beta->value();
}

void Interface::set_prior_lambda()
{
	model->params->prior_lambda = ui.doubleSpinBox_prior_lambda->value();
}

void Interface::set_split_params()
{
	model->params->split_params.length_thresh = max(ui.doubleSpinBox_split_length_thresh->value(), model->params->split_params.length_thresh);
}

void Interface::do_lsd()
{
	if (path_background.toStdString().c_str() == 0) return;

	if (!thread_busy) {
		thread_busy = true;

		ui.label_process->setText(QString("Running LSD..."));
		ui.label_message->setText("");
		ui.label_message->setStyleSheet("QLabel { }");
		
		thread_master.extract_segments_async(kinetic_partition, model);
	}
}


void Interface::handle_lsd_done()
{
	thread_busy = false;

	ui.label_process->setText("");
	ui.label_message->setText(QString("Segments : %1").arg(model->segments.size()));
	
	unload_partitioning_products();
	load_partitioning_products();

	ui.checkBox_display_background->setChecked(true);
	ui.checkBox_display_input_prediction->setChecked(false);
	ui.checkBox_display_partition->setChecked(false);

	set_visibility_layer_background();
	set_visibility_layer_input_prediction();
	set_visibility_layer_partition();
	set_visibility_layer_next_operation();

	ui.graphicsView->show();
}

void Interface::do_evolution()
{
	if (!thread_busy) {
		thread_busy = true;
		unload_partitioning_products();

		/* Initialize */
		if (model->graph == NULL) {
			ui.label_process->setText(QString("Step 1/%1 : Running LSD...").arg(model->params->rega_regp_enabled ? 3 : 2));
			ui.label_message->setText("");
			ui.label_message->setStyleSheet("QLabel { }");
			ui.pushButton_save->setEnabled(false);

			start_time = clock();
			thread_master.generate_kinetic_partition_async(kinetic_partition, model);
		}
		
		/* Refine */
		ui.label_process->setText("Merging and splitting...");
		ui.pushButton_save->setEnabled(false);

		start_time = clock();
		thread_master.merge_and_split_async(kinetic_partition, model);
	}
}

void Interface::reset_graph()
{
	delete model->graph;
	model->graph = NULL;
	
	unload_partitioning_products();
	unload_highlighted_facets();

	set_iterations();
	set_alignment_beta();
	set_prior_lambda();
	set_split_params();
}

void Interface::handle_lsd_done_as_intermediate_step()
{
	if (model->params->rega_regp_enabled) {
		ui.label_process->setText(QString("Step 2/%1 : Regularizing segments...").arg(model->params->rega_regp_enabled ? 3 : 2));
	}
	else {
		ui.label_process->setText(QString("Step 2/%1 : Propagating rays...").arg(model->params->rega_regp_enabled ? 3 : 2));
	}
}


void Interface::handle_regularization_done_as_intermediate_step()
{
	ui.label_process->setText("Step 3/3 : Propagating rays...");
}


void Interface::handle_propagation_done_as_intermediate_step()
{
	ui.label_process->setText("Loading results, please wait...");
}

void Interface::handle_preprocessing_done_as_intermediate_step()
{
	ui.label_process->setText("Preprocessing done, entering next step...");
}


void Interface::handle_generation_done()
{
	thread_busy = false;
	end_time = clock();

	double elapsed_time = double(end_time - start_time) / CLOCKS_PER_SEC;
	int cells = int(model->graph->faces.size());
	ui.label_process->setText("");
	ui.label_message->setText(QString("Initialized %1 cells in %2 s. Now waiting for iterations to finish...")
		.arg(cells).arg(elapsed_time, 0, 'g', 2));
	ui.label_message->setStyleSheet("QLabel { }");

	unload_partitioning_products();
	load_partitioning_products();

	ui.checkBox_display_background->setChecked(true);
	ui.checkBox_display_input_prediction->setChecked(false);

	ui.checkBox_display_split_segments->setChecked(false);
	ui.checkBox_display_partition->setChecked(true);
	ui.checkBox_display_next_operation->setChecked(false);

	set_visibility_layer_background();
	set_visibility_layer_input_prediction();
	set_visibility_layer_split_segments();
	set_visibility_layer_partition();
	set_visibility_layer_next_operation();

	ui.graphicsView->show();

	ui.pushButton_save->setEnabled(true);
}

void Interface::handle_evolution_done()
{
	thread_busy = false;
	end_time = clock();

	ui.label_process->setText("");
	update_label_cells_count();

	update_evolution_products();

	ui.checkBox_display_background->setChecked(true);
	ui.checkBox_display_partition->setChecked(true);
	ui.checkBox_display_next_operation->setChecked(true);

	set_visibility_layer_background();
	set_visibility_layer_input_prediction();
	set_visibility_layer_split_segments();
	set_visibility_layer_partition();
	set_visibility_layer_next_operation();

	ui.graphicsView->show();

	ui.pushButton_save->setEnabled(true);
}

void Interface::update_label_cells_count()
{
	double elapsed_time = double(end_time - start_time) / CLOCKS_PER_SEC;
	int cells = int(model->graph->faces.size());

	ui.label_message->setText(QString("Built %1 cells in %2 s.")
		.arg(cells).arg(elapsed_time, 0, 'g', 2));
	ui.label_message->setStyleSheet("QLabel { }");
}


void Interface::update_evolution_products()
{
	unload_qgraphics_line_items(layer_partition);
	unload_qgraphics_line_items(layer_next_operation);
	unload_qgraphics_line_items(layer_split_segments);
	unload_highlighted_facets();
	unload_colored_facets();

	load_partitioning_products();
	load_highlighted_facets();
}


void Interface::load_partitioning_products()
{
	load_qgraphics_line_items(model->L_prop, layer_partition, ui.checkBox_display_partition, 2.0);
	load_qgraphics_line_items(model->L_next_operation, layer_next_operation, ui.checkBox_display_partition, 3.0);
	load_qgraphics_line_items(model->L_split_segments, layer_split_segments, ui.checkBox_display_partition, 3.5);
}


void Interface::unload_partitioning_products()
{
	unload_qgraphics_line_items(layer_partition);
	unload_qgraphics_line_items(layer_next_operation);
	unload_qgraphics_line_items(layer_regularized);
	unload_qgraphics_line_items(layer_split_segments);
	unload_colored_facets();
	unload_highlighted_facets();

	ui.checkBox_display_input_prediction->setChecked(false);
	ui.checkBox_display_partition->setChecked(false);
	ui.checkBox_display_next_operation->setChecked(false);
	ui.checkBox_display_split_segments->setChecked(false);
	ui.checkBox_display_colored_facets->setChecked(false);
}

void Interface::load_highlighted_facets()
{
	QImage highlighted_facets;
	Matrix<uchar> J(model->I.rows, model->I.cols, 4);
	model->graph->draw_highlighted_facets(J);
	load_matrix_argb_as_qimage(J, highlighted_facets);
	load_qimage_as_qgraphicspixmapitem(highlighted_facets, layer_highlighted_facets, ui.checkBox_display_next_operation, 1., 0.4);
}


void Interface::unload_highlighted_facets()
{
	if (layer_highlighted_facets != NULL) {
		scene.removeItem(layer_highlighted_facets);
		delete layer_highlighted_facets;
	}
	layer_highlighted_facets = NULL;
}


void Interface::load_colored_facets()
{
	Matrix<uchar> J(model->I.rows, model->I.cols, 3);
	model->graph->draw_faces(J);
	load_matrix_argb_as_qimage(J, qimage_colored_facets);
	load_qimage_as_qgraphicspixmapitem(qimage_colored_facets, layer_colored_facets, ui.checkBox_display_colored_facets, 1., ui.doubleSpinBox_colored_facets_alpha->value());
}


void Interface::unload_colored_facets()
{
	if (layer_colored_facets != NULL) {
		scene.removeItem(layer_colored_facets);
		delete layer_colored_facets;
	}
	qimage_colored_facets = QImage();
	layer_colored_facets = NULL;
}


void Interface::set_visibility_layer_background()
{
	if (layer_background != NULL) layer_background->setVisible(ui.checkBox_display_background->isChecked());
}

void Interface::set_visibility_layer_input_prediction()
{
	if (layer_input_prediction != NULL) layer_input_prediction->setVisible(ui.checkBox_display_input_prediction->isChecked());
}

void Interface::set_visibility_layer_split_segments()
{
	if (ui.checkBox_display_split_segments->isChecked()) {
		show_qgraphics_line_items(layer_split_segments);
	}
	else {
		hide_qgraphics_line_items(layer_split_segments);
	}
}


void Interface::set_visibility_layer_partition()
{
	if (ui.checkBox_display_partition->isChecked()) {
		show_qgraphics_line_items(layer_partition);
	} else {
		hide_qgraphics_line_items(layer_partition);
	}
}

void Interface::set_visibility_layer_next_operation()
{
	if (ui.checkBox_display_next_operation->isChecked()) {
		show_qgraphics_line_items(layer_next_operation);		
	}
	else {
		hide_qgraphics_line_items(layer_next_operation);
	}
	if (layer_highlighted_facets != NULL) layer_highlighted_facets->setVisible(ui.checkBox_display_next_operation->isChecked());
}

void Interface::set_visibility_layer_colored_facets()
{
	if (layer_colored_facets == NULL) {
		load_colored_facets();
	}
	layer_colored_facets->setVisible(ui.checkBox_display_colored_facets->isChecked());
}


void Interface::set_semantic_type(Semantic_Type type)
{
	model->params->semantic_type = type;
}
