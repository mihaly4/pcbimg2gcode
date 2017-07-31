#ifndef IMG2GCODE_H
#define IMG2GCODE_H

#include "basetask.h"

class QImage;
class Img2Gcode : public BaseTask
{
    Q_OBJECT

    void GenerateLine(int y);
    QString MoveTo(int x, int y);
    void EmitLine(int iStart, int y, int iEnd);

public:
    explicit Img2Gcode(const QStringList & lArgs, QObject *parent = nullptr);

public slots:
    void run();

signals:

};

#endif // IMG2GCODE_H
