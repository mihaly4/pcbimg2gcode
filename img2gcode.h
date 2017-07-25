#ifndef IMG2GCODE_H
#define IMG2GCODE_H

#include <QObject>

class QImage;
class Img2Gcode : public QObject
{
    Q_OBJECT

    int         m_iImgDpi;
    QImage *    m_pSrcImage;
    QString     m_sImgFileName;
    QString     m_sGcodeFileName;
    QStringList m_lGcode;

    void InitializePrint();
    void FinilizePrint();
    void GenerateLine(const uchar * pScanLine);

public:
    explicit Img2Gcode(const QStringList & lArgs, QObject *parent = nullptr);
    ~Img2Gcode();

public slots:
    void run();

signals:
    void finished();
};

#endif // IMG2GCODE_H
