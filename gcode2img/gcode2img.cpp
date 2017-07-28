#include "gcode2img.h"
#include <QImage>
#include <QDebug>
#include <QFile>
#include <math.h>

#define SIGN(a) (((a) < 0) ? -1 : 1)

#define INCH2MM 25.4
#define Y_START -40.0
#define X_START -40.0

void GCode2Img::MoveLaser(bool bLaserOn, double fX, double fY)
{
    int iXStart = (m_fCurrentX)/ INCH2MM * m_fImgDpiX;
    int iXEnd = (fX)/ INCH2MM * m_fImgDpiX;
    int iY = round((fY)/ INCH2MM * m_fImgDpiY);
    int iStep = SIGN(iXEnd - iXStart);
    if(bLaserOn)
    {
        for(int x = iXStart; x != (iXEnd + iStep); x += iStep)
        {
            m_pSrcImage->setPixel(x, iY, 0xffffffff);
        }
    }
    m_fCurrentX = fX;
    m_fCurrentY = fY;
}

GCode2Img::GCode2Img(const QStringList &lArgs, QObject *parent) : QObject(parent)
{
    m_sGcodeFileName = lArgs[0];
    m_sImgFileName = lArgs[1];
    m_pSrcImage = new QImage(lArgs[2].toInt(), lArgs[3].toInt(), QImage::Format_RGB888);
    m_fCurrentX = 0;
    m_fCurrentY = 0;
    m_fImgDpiX = lArgs[4].toFloat();
    m_fImgDpiY = lArgs[4].toFloat();
}

void GCode2Img::run()
{
    QFile tInputFile(m_sGcodeFileName);
    if(tInputFile.open(QFile::ReadOnly))
    {
        m_lGcode = QString::fromUtf8(tInputFile.readAll()).split("\n");
    }
    bool bLaserOn = false;

    for(int i = 0; i < m_lGcode.count(); i++)
    {
        QString sCommand = m_lGcode[i];
        QStringList lTokens = sCommand.split(" ", QString::SkipEmptyParts);
        if(lTokens.count() > 0)
        {
            if(lTokens[0] == "G1")
            {
                double fNewY = m_fCurrentY;
                double fNewX = m_fCurrentX;
                for(int t = 1; t < lTokens.count(); t++)
                {
                    if(lTokens[t].startsWith("X"))
                        fNewX = lTokens[t].mid(1).toFloat() - X_START;
                    else if(lTokens[t].startsWith("Y"))
                        fNewY = lTokens[t].mid(1).toFloat() - Y_START;
                }
                MoveLaser(bLaserOn, fNewX, fNewY);
            }
            if(lTokens[0] == "M42")
            {
                for(int t = 1; t < lTokens.count(); t++)
                {
                    if(lTokens[t].startsWith("S"))
                        bLaserOn = (lTokens[t].mid(1).toInt() == 255);
                }
            }
        }
    }
    m_pSrcImage->save(m_sImgFileName);
    delete m_pSrcImage;
    qDebug() << "Done!";
    emit finished();
}
