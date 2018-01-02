#include "img2gcode.h"
#include <QImage>
#include <QFile>
#include <QDebug>

#define INCH2MM 25.4
#define Y_START -40.0
#define X_START -40.0
#define CORRECTION 0.961538461538
#define START_DELAY_G_CODE "G4 P20"
#define STOP_DELAY_G_CODE "G4 P5"
//#define FORCE_LEFT_TO_RIGHT
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
            << START_DELAY_G_CODE
            << ("M42 P" + m_sLaserPin + " S255")
            << MoveTo(iEnd, y)
            << STOP_DELAY_G_CODE
            << ("M42 P" + m_sLaserPin + " S0");

}

void Img2Gcode::GenerateLine(int y)
{
    bool bTraceOn = false;
#ifdef FORCE_LEFT_TO_RIGHT
    int iTraceStart = 0;
    int iXStep = 1;
    int iForceTraceEndX = m_pSrcImage->width() - 1;
#else
    int iTraceStart = (y % 2 == 0 ? 0 : m_pSrcImage->width() - 1);
    int iXStep = (y % 2) == 0 ? 1 : -1;
    int iForceTraceEndX = (y % 2== 0) ? m_pSrcImage->width() - 1 : 0;
#endif

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
    return QString() + "G1 X" +
            QString::number((x / m_fImgDpiX * INCH2MM + X_START) * CORRECTION) +
            " Y" +
            QString::number((y / m_fImgDpiY * INCH2MM + Y_START) * CORRECTION);
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

void Img2Gcode::WriteGcode(const QString & sFileName)
{
    qDebug() << "Writing: " << sFileName;
    QFile tOutFile(sFileName);
    if(tOutFile.open(QFile::WriteOnly | QFile::Text))
    {
        tOutFile.write(m_lGcode.join("\n").toUtf8());
        tOutFile.close();
    }
}

void Img2Gcode::run()
{
    m_pSrcImage = new QImage(m_sImgFileName);
    m_fImgDpiX = m_pSrcImage->dotsPerMeterX() * INCH2MM / 1000.0f;
    m_fImgDpiY = m_pSrcImage->dotsPerMeterY() * INCH2MM / 1000.0f;
    qDebug() << "Image dpi is "<< m_fImgDpiX << "x" << m_fImgDpiY;
    qDebug() << "PCB dimensions are "
             << QString::number((m_pSrcImage->width() / m_fImgDpiX * INCH2MM))
             << "x"
             << QString::number((m_pSrcImage->height() / m_fImgDpiY * INCH2MM));
    InitializePrint();
    m_lGcode << ("G1 X"+QString::number(X_START * CORRECTION)+" Y"+QString::number(Y_START * CORRECTION));
    m_lGcode << ("M42 P" + m_sLaserPin + " S255");
    m_lGcode << ("G1 X"+QString::number(X_START * CORRECTION)+" Y" + QString::number((m_pSrcImage->height() / m_fImgDpiY * INCH2MM + Y_START) * CORRECTION));
    m_lGcode << ("G1 X"+QString::number((m_pSrcImage->width() / m_fImgDpiY * INCH2MM + Y_START) * CORRECTION)+" Y" + QString::number((m_pSrcImage->height() / m_fImgDpiY * INCH2MM + Y_START) * CORRECTION));
    m_lGcode << ("G1 X"+QString::number((m_pSrcImage->width() / m_fImgDpiY * INCH2MM + Y_START) * CORRECTION)+" Y" + QString::number(Y_START * CORRECTION));
    m_lGcode << ("G1 X"+QString::number(X_START * CORRECTION)+" Y"+QString::number(Y_START * CORRECTION));
    m_lGcode << ("M42 P" + m_sLaserPin + " S0");
    FinilizePrint();
    WriteGcode("/home/mihaly4/rect.gcode");


    m_lGcode.clear();
    InitializePrint();
    for(int y = 0; y < m_pSrcImage->height(); y++)
    {
        GenerateLine(y);
    }
    FinilizePrint();
    delete m_pSrcImage;
    WriteGcode(m_sGcodeFileName);
    qDebug() << "Done";
    emit finished();
}
