#include "img2gcode.h"
#include <QImage>
#include <QFile>
#include <QDebug>

#define INCH2MM 25.4f
#define Y_START 50.0f
void Img2Gcode::InitializePrint()
{
    m_lGcode
                <<"; Default start code"
                <<"G28 ; Home extruder"
                <<"G1 Z15 F2000"
                <<"M107 ; Turn off fan"
                <<"G90 ; Absolute positioning"
                <<"G1 Z40.00";
}

void Img2Gcode::FinilizePrint()
{
    m_lGcode << "G28";
}

void Img2Gcode::GenerateLine(int y)
{
    bool bTraceOn = false;
    int iTraceStart = 0;
    for(int x = 0; x < m_pSrcImage->width(); x++)
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
                m_lGcode
                        << MoveTo(iTraceStart, y)
                        << ("M42 P" + m_sLaserPin + " S255")
                        << MoveTo(x-1, y)
                        << ("M42 P" + m_sLaserPin + " S0");
            }
        }
    }
}

QString Img2Gcode::MoveTo(int x, int y)
{
    return QString() + "G1 X" + QString::number((x - m_pSrcImage->width() / 2) / (float)m_iImgDpi * INCH2MM) + " Y" + QString::number(y / (float)m_iImgDpi * INCH2MM - Y_START);
}

Img2Gcode::Img2Gcode(const QStringList &lArgs, QObject *parent) : QObject(parent)
{
    m_iImgDpi = lArgs[0].toInt();
    m_sImgFileName = lArgs[1];
    m_sGcodeFileName = lArgs[2];
    m_pSrcImage = NULL;
    m_sLaserPin = lArgs[3];
}

Img2Gcode::~Img2Gcode()
{

}

void Img2Gcode::run()
{
    m_pSrcImage = new QImage(m_sImgFileName);
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
