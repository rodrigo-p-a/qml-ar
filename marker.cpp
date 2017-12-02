#include "marker.h"
#include <QTextStream>

void Marker::setPositionMM(QVector2D position_mm_)
{
    position_mm = position_mm_;
}

Marker::Marker()
{
    setPositionMM(QVector2D(0, 0));
    resetH();
}

Marker::Marker(double x_mm, double y_mm)
{
    setPositionMM(QVector2D(x_mm, y_mm));
}

Marker::Marker(QVector2D position_mm_)
{
    setPositionMM(position_mm_);
}

void Marker::setName(QString name_)
{
    name = name_;
}

void Marker::setId(int id_)
{
    id = id_;
}

void Marker::setSizeMM(double size_)
{
    size_mm = size_;
}

const QVector2D Marker::getPositionMM()
{
    return position_mm;
}

double Marker::getSizeMM()
{
    return size_mm;
}

void Marker::setH(QMatrix4x4 H_)
{
    H = H_;
}

QMatrix4x4 Marker::getH()
{
    return H;
}

void Marker::resetH()
{
    //H.fill(0.0f);
    H = QMatrix4x4();
}

QString Marker::toString()
{
    QString res;
    QTextStream ts(&res);
    ts << "Marker [" << name << "] #" << id << " position ("
       << position_mm.x() << ", " << position_mm.y() << ") mm"
       << " size " << size_mm;
    return(res);
}
