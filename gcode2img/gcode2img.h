#ifndef GCODE2IMG_H
#define GCODE2IMG_H

#include <QObject>

class GCode2Img : public QObject
{
    Q_OBJECT

    double       m_fImgDpiX;
    double       m_fImgDpiY;
    QImage *    m_pSrcImage;
    QString     m_sImgFileName;
    QString     m_sGcodeFileName;
    QStringList m_lGcode;
    double       m_fCurrentX;
    double       m_fCurrentY;

    void        MoveLaser(bool bLaserOn, double fX, double fY);
public:
    explicit GCode2Img(const QStringList & lArgs, QObject *parent = nullptr);

public slots:
    void run();

signals:
    void finished();
};

#endif // GCODE2IMG_H
