#include "img2gcode.h"
#include <QImage>
#include <QFile>
void Img2Gcode::InitializePrint()
{
    //TODO: Add initialization code
}

void Img2Gcode::FinilizePrint()
{
    //TODO: Add finilization code
}

void Img2Gcode::GenerateLine(const uchar * pScanLine)
{
    //TODO: Add line generation code
}

Img2Gcode::Img2Gcode(const QStringList &lArgs, QObject *parent) : QObject(parent)
{
    m_iImgDpi = lArgs[0].toInt();
    m_sImgFileName = lArgs[1];
    m_sGcodeFileName = lArgs[2];
    m_pSrcImage = NULL;
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
        GenerateLine(m_pSrcImage->scanLine(y));
    }
    FinilizePrint();
    delete m_pSrcImage;
    QFile tOutFile(m_sGcodeFileName);
    if(tOutFile.open(QFile::WriteOnly | QFile::Text))
    {
        tOutFile.write(m_lGcode.join("\n").toUtf8());
        tOutFile.close();
    }
    emit finished();
}
