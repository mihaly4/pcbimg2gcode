#include "img2gcode.h"
#include <QImage>
#include <QFile>
#include <QDebug>

#define INCH2MM 25.4f
#define Y_START -40.0f
#define X_START -40.0f
void Img2Gcode::InitializePrint()
{
    m_lGcode
                << "; Default start code"
                << ("M42 P" + m_sLaserPin + " S0")
                << "G28 ; Home extruder"
                << "G1 Z15 F2000"
                << "M107 ; Turn off fan"
                << "G90 ; Absolute positioning"
                << "G1 Z40.00";
}

void Img2Gcode::FinilizePrint()
{
    m_lGcode << "G28";
}

void Img2Gcode::EmitLine(int iStart, int y, int iEnd)
{
    m_lGcode
            << MoveTo(iStart, y)
            << "G4 P30"
            << ("M42 P" + m_sLaserPin + " S255")
            << MoveTo(iEnd, y)
            << ("M42 P" + m_sLaserPin + " S0")
            << "G4 P30";
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

Img2Gcode::Img2Gcode(const QStringList &lArgs, QObject *parent) : QObject(parent)
{
    m_fImgDpiX = 400;
    m_fImgDpiY = 400;
    m_sImgFileName = lArgs[0];
    m_sGcodeFileName = lArgs[1];
    m_pSrcImage = NULL;
    m_sLaserPin = lArgs[2];
}

Img2Gcode::~Img2Gcode()
{

}

void Img2Gcode::run()
{
    m_pSrcImage = new QImage(m_sImgFileName);
    m_fImgDpiX = m_pSrcImage->dotsPerMeterX() * INCH2MM / 1000.0f;
    m_fImgDpiY = m_pSrcImage->dotsPerMeterY() * INCH2MM / 1000.0f;
    m_lGcode.clear();
    InitializePrint();
    for(int y = 0; y < m_pSrcImage->height(); y++)
    {
        GenerateLine(y);
    }
    FinilizePrint();
    delete m_pSrcImage;
    QFile tOutFile(m_sGcodeFileName);
    if(tOutFile.open(QFile::WriteOnly | QFile::Text))
    {
        tOutFile.write(m_lGcode.join("\n").toUtf8());
        tOutFile.close();
    }
    qDebug() << "Done";
    emit finished();
}
