#include "img2gcode.h"
#include <QImage>
#include <QFile>
#include <QDebug>
#include "common.h"

void Img2Gcode::EmitLine(int iStart, int y, int iEnd)
{
    if(m_iOutputFormat == FORMAT_GCODE)
    {
        m_lGcode
                << MoveTo(iStart, y)
                << START_DELAY_G_CODE
                << ("M42 P" + m_sLaserPin + " S255")
                << MoveTo(iEnd, y)
                << STOP_DELAY_G_CODE
                << ("M42 P" + m_sLaserPin + " S0");
    }
    else
    {
        m_lGcode << ("PA"+PixelToHpgl(QPoint(iStart, y)) + ";PD;PA" + PixelToHpgl(QPoint(iEnd, y)) + ";PU");
        //m_lGcode << ("PU;"+MoveTo(iStart, y)+";PD;"+MoveTo(iEnd, y)+";PU");
    }

}

void Img2Gcode::EmitLineX(int iStart, int x, int iEnd)
{
    if(m_iOutputFormat == FORMAT_GCODE)
    {
        m_lGcode
                << MoveTo(x, iStart)
                << START_DELAY_G_CODE
                << ("M42 P" + m_sLaserPin + " S255")
                << MoveTo(x, iEnd)
                << STOP_DELAY_G_CODE
                << ("M42 P" + m_sLaserPin + " S0");
    }
    else
    {
        m_lGcode << ("PA"+PixelToHpgl(QPoint(x, iStart)) + ";PD;PA" + PixelToHpgl(QPoint(x, iEnd)) + ";PU");
    }
}

void Img2Gcode::GenerateLine(int y)
{
    bool bTraceOn = false;
    int iTraceStart = (y % 2 == 0 ? 0 : m_pSrcImage->width()-1);
    int iXStep = (y % 2) == 0 ? 1 : -1;
    int iForceTraceEndX = (y % 2== 0) ? m_pSrcImage->width()-1 : 0;

    for(int x = iTraceStart; x != (iForceTraceEndX + iXStep); x += iXStep)
    {
        bool bNewTraceOn = QColor(m_pSrcImage->pixel(x,y)).blue() > 20 || QColor(m_pSrcImage->pixel(x,y)).green() > 20;
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

void Img2Gcode::GenerateLineX(int x)
{
    bool bTraceOn = false;
    int iTraceStart = (x % 2 == 0 ? 0 : m_pSrcImage->height()-1);
    int iYStep = (x % 2) == 0 ? 1 : -1;
    int iForceTraceEndY = (x % 2== 0) ? m_pSrcImage->height()-1 : 0;

    for(int y = iTraceStart; y != (iForceTraceEndY + iYStep); y += iYStep)
    {
        bool bNewTraceOn = QColor(m_pSrcImage->pixel(x,y)).blue() > 20;
        if(bNewTraceOn != bTraceOn)
        {
            bTraceOn = bNewTraceOn;
            if(bTraceOn)
            {
                iTraceStart = y;
            }
            else
            {
                EmitLineX(iTraceStart, x, y - iYStep);
            }
        }
    }
    if(bTraceOn)
    {
        EmitLineX(iTraceStart, x, iForceTraceEndY);
    }
}

QString Img2Gcode::MoveTo(int x, int y)
{
    return QString() + "PA" + QString::number(x / m_fImgDpiX * INCH2MM + X_START) + "," + QString::number(y / m_fImgDpiY * INCH2MM + Y_START);
}

Img2Gcode::Img2Gcode(const QStringList &lArgs, bool bXscan, QObject *parent) : BaseTask(lArgs, parent)
{
    m_bXlines = bXscan;
}





void Img2Gcode::run()
{
    InitializePrint();
    if(m_bXlines)
    {
        for(int x = 0; x < m_pSrcImage->width(); x++)
        {
            GenerateLineX(x);
        }
    }
    else
    {
        for(int y = 0; y < m_pSrcImage->height(); y++)
        {
            GenerateLine(y);
        }
    }
    FinilizePrint();
    WriteGcode();
    qDebug() << "Done";
    emit finished();
}
