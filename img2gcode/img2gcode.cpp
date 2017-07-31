#include "img2gcode.h"
#include <QImage>
#include <QFile>
#include <QDebug>
#include "common.h"

void Img2Gcode::EmitLine(int iStart, int y, int iEnd)
{
    m_lGcode
            << MoveTo(iStart, y)
            << START_DELAY_G_CODE
            << ("M42 P" + m_sLaserPin + " S255")
            << MoveTo(iEnd, y)
            << STOP_DELAY_G_CODE
            << ("M42 P" + m_sLaserPin + " S0");

}

void Img2Gcode::GenerateLine(int y)
{
    bool bTraceOn = false;
    int iTraceStart = (y % 2 == 0 ? 0 : m_pSrcImage->width()-1);
    int iXStep = (y % 2) == 0 ? 1 : -1;
    int iForceTraceEndX = (y % 2== 0) ? m_pSrcImage->width()-1 : 0;

    for(int x = iTraceStart; x != (iForceTraceEndX + iXStep); x += iXStep)
    {
        bool bNewTraceOn = QColor(m_pSrcImage->pixel(x,y)).blue() > 20;
        if(bNewTraceOn != bTraceOn)
        {
            bTraceOn = bNewTraceOn;
            if(bTraceOn)
            {
                iTraceStart = x;
            }
            else
            {
                EmitLine(iTraceStart, y, x - iXStep);
            }
        }
    }
    if(bTraceOn)
    {
        EmitLine(iTraceStart, y, iForceTraceEndX);
    }
}

QString Img2Gcode::MoveTo(int x, int y)
{
    return QString() + "G1 X" + QString::number(x / m_fImgDpiX * INCH2MM + X_START) + " Y" + QString::number(y / m_fImgDpiY * INCH2MM + Y_START);
}

Img2Gcode::Img2Gcode(const QStringList &lArgs, QObject *parent) : BaseTask(lArgs, parent)
{

}





void Img2Gcode::run()
{
    InitializePrint();
    for(int y = 0; y < m_pSrcImage->height(); y++)
    {
        GenerateLine(y);
    }
    FinilizePrint();
    WriteGcode();
    qDebug() << "Done";
    emit finished();
}
