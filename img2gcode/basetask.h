#ifndef BASETASK_H
#define BASETASK_H

#include <QObject>


class BaseTask : public QObject
{
    Q_OBJECT

protected:
    QString     m_sImgFileName;
    QString     m_sGcodeFileName;
    QString     m_sLaserPin;
    QImage *    m_pSrcImage;
    float       m_fImgDpiX;
    float       m_fImgDpiY;
    QStringList m_lGcode;

    void InitializePrint();
    void FinilizePrint();
    void WriteGcode();
public:
    explicit BaseTask(const QStringList &lArgs, QObject *parent = nullptr);
    ~BaseTask();

public slots:
    virtual void run() = 0;

signals:
    void finished();
};

#endif // BASETASK_H
