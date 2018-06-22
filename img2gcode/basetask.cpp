#include "basetask.h"
#include "common.h"
#include <QImage>
#include <QFile>

void BaseTask::InitializePrint()
{
    m_lGcode.clear();
    if(m_iOutputFormat == FORMAT_GCODE)
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
    else
    {
        //m_lGcode << "IN";
    }
}

void BaseTask::FinilizePrint()
{
    if(m_iOutputFormat == FORMAT_GCODE)
    {
        m_lGcode << "G28";
    }
    //else
        //m_lGcode << "PU";
}

void BaseTask::WriteGcode()
{
    QFile tOutFile(m_sGcodeFileName);
    if(tOutFile.open(QFile::WriteOnly | QFile::Text))
    {
        if(m_iOutputFormat == FORMAT_GCODE)
        {
            tOutFile.write(m_lGcode.join("\n").toUtf8());
        }
        else
        {
            tOutFile.write(m_lGcode.join(";\n").toUtf8());
        }
        tOutFile.close();
    }
}

BaseTask::BaseTask(const QStringList &lArgs, QObject *parent) : QObject(parent)
{
    m_sImgFileName = lArgs[0];
    m_sGcodeFileName = lArgs[1];
    m_sLaserPin = lArgs[2];
    m_iOutputFormat = FORMAT_HPLG;
    m_pSrcImage = new QImage(m_sImgFileName);
    m_fImgDpiX = m_pSrcImage->dotsPerMeterX() * INCH2MM / 1000.0f;
    m_fImgDpiY = m_pSrcImage->dotsPerMeterY() * INCH2MM / 1000.0f;
    m_lGcode.clear();
}

BaseTask::~BaseTask()
{
    if(m_pSrcImage != NULL)
        delete m_pSrcImage;
}
