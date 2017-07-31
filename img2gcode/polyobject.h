#ifndef POLYOBJECT_H
#define POLYOBJECT_H

#include <QObject>
#include <QVector>
#include <QPointF>

class PolyObject : public QObject
{
    Q_OBJECT
    QVector<QPointF>    m_lPoints;
public:
    explicit PolyObject(QObject *parent = nullptr);

    QString GetGcode();
signals:

public slots:
};

#endif // POLYOBJECT_H
