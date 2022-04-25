#ifndef MYQGRAPHICSVIEW_H
#define MYQGRAPHICSVIEW_H

#include <QObject>
#include <QGraphicsView>
#include <QMouseEvent>


class MyQGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    MyQGraphicsView(QWidget *parent = 0);

    void mousePressEvent(QMouseEvent* event);

    void mouseMoveEvent(QMouseEvent* event);

    void mouseReleaseEvent(QMouseEvent* event);

    void wheelEvent(QWheelEvent* event);

    std::list<std::pair<int, int> > points;

	bool mousePressed;

signals:

	void mouseReleased(bool);
};

#endif // MYQGRAPHICSVIEW_H
