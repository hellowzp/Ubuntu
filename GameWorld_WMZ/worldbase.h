#ifndef WORLDBASE_H
#define WORLDBASE_H

#include <QGraphicsRectItem>
#include "world.h"
#include "mainwindow.h"
#include <QObject>

class WorldBase :public QObject, public QGraphicsRectItem, public Tile
{
    Q_OBJECT
public:  
    explicit WorldBase(const Tile & tile);
    ~WorldBase();
    void setColor(int color);
public slots:
    void setHightlight(const QColor &color);
    void unsetHightlight();
private:
    QColor _color;

};

#endif // WORLDBASE_H

