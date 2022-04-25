#include "myqgraphicsview.h"
#include <iostream>
#include <math.h>


MyQGraphicsView::MyQGraphicsView(QWidget *parent)
	: QGraphicsView(parent),
	mousePressed (false)
{
    viewport()->installEventFilter(this);
}


void MyQGraphicsView::mousePressEvent(QMouseEvent* event)
{
    QPointF pt = mapToScene(event->pos());
	int x = int(std::floor(pt.x())), y = int(std::floor(pt.y()));
	std::pair<int, int> p(x, y);
    points.push_back(p);
	mousePressed = true;
}


void MyQGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
	if (mousePressed) {
		QPointF pt = mapToScene(event->pos());
		int x = int(std::floor(pt.x())), y = int(std::floor(pt.y()));
		std::pair<int, int> p(x, y);
		points.push_back(p);
	}
}


void MyQGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
	mousePressed = false;
    if (event->button() == Qt::MouseButton::LeftButton) {
		emit mouseReleased(true);
	} else if (event->button() == Qt::MouseButton::RightButton) {
		emit mouseReleased(false);
	}
}


void MyQGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) {
        scale(1.25, 1.25);
    } else {
        scale(0.8, 0.8);
    }
}

