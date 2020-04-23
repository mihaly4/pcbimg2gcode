#include "grb2vectors.h"
#include <QDebug>
#include <QFile>
#include <QPoint>
#include <QPointF>
#include <math.h>
#include <QRectF>
#include "common.h"

grb2vectors::grb2vectors(const QStringList &lArgs, QObject *parent) : BaseTask(lArgs, parent)
{

}

grb2vectors::~grb2vectors()
{

}

QRectF BoundingBox(const QVector<QPointF> & lLine)
{
    QRectF tBox = QRectF(lLine.first(),lLine.first());
    for(int i = 1; i < lLine.count(); i++)
    {
        tBox.setLeft(std::min(tBox.left(),lLine[i].x()));
        tBox.setRight(std::max(tBox.right(),lLine[i].x()));

        tBox.setTop(std::min(tBox.top(),lLine[i].y()));
        tBox.setBottom(std::max(tBox.bottom(),lLine[i].y()));
    }

    return tBox;
}
float Length(const QPointF & tVector)
{
    return sqrtf(tVector.x() * tVector.x() + tVector.y() * tVector.y());
}

QPointF Normalize(const QPointF & tVector)
{
    float tLength = sqrtf(tVector.x() * tVector.x() + tVector.y() * tVector.y());
    return QPointF(tVector.x() /  tLength, tVector.y() /  tLength);
}

QPointF Scale(const QPointF & tVector, const QPointF & tCenter, float fScale)
{
    return tCenter + (tVector - tCenter) * fScale;
}

QPointF Rotate90(const QPointF & tVector)
{
    return QPointF(tVector.y(),-tVector.x());
}
enum CONTOUR_TYPE
{
    CONTOUR_LINE,
    CONTOUR_CIRCLE,
    CONTOUR_RECT
};

struct contour_t
{
    QVector<QPointF> m_lPoints;
    CONTOUR_TYPE m_iType;
};

void grb2vectors::EmitLines()
{
    for(int t = 0; t < m_lTracks.count(); t++)
    {
        const auto & m_lLines = m_lTracks[t].m_lLines;
        for(int i = 0; i < m_lLines.count(); i++)
        {
            const QVector<QPointF> & lLine = m_lLines[i];
            if(lLine.size())
            {
                QRectF tBox = BoundingBox(lLine);
                if(m_lTracks[t].m_bFill == false)//if(lLine.count() == 2 && lLine[0] == lLine[1])
                {
                    switch(m_lTracks[t].m_tTool.iType)
                    {
                    case CIRCLE:
                        tBox = QRectF(lLine[0] - QPointF(m_lTracks[t].m_tTool.s1,m_lTracks[t].m_tTool.s1),lLine[0] + QPointF(m_lTracks[t].m_tTool.s1,m_lTracks[t].m_tTool.s1));
                        break;
                    case OBROUND:
                        tBox = QRectF(lLine[0] - QPointF(m_lTracks[t].m_tTool.s1,m_lTracks[t].m_tTool.s2),lLine[0] + QPointF(m_lTracks[t].m_tTool.s1,m_lTracks[t].m_tTool.s2));
                        break;
                    case RECTANGLE:
                        tBox = QRectF(lLine[0] - QPointF(m_lTracks[t].m_tTool.s1,m_lTracks[t].m_tTool.s2),lLine[0] + QPointF(m_lTracks[t].m_tTool.s1,m_lTracks[t].m_tTool.s2));
                        break;
                    }
                }
                float tMaxDim = std::max(tBox.width(), tBox.height());
                QPointF tCenter = tBox.center();
                int iNumScales = 1 + tMaxDim / 3.0f;
                float fStepSale = 1.0f / iNumScales;

                if(m_lTracks[t].m_bFill == false)
                {

                    for(int sc =0; sc <= iNumScales; sc++)
                    {
                        float fScale = fStepSale * sc;

                        QVector<contour_t> lContours;
                        contour_t lContour;
                        lContour.m_iType = CONTOUR_LINE;
                        //Contour forward
                        bool bAddLineCap = false;
                        QPointF tLineCapStart;
                        for(int  j = 1; j < lLine.count(); j++)
                        {
                            QPointF tLineVector = lLine[j] - lLine[j - 1];
                            if(Length(tLineVector) == 0)
                            {
                                //Add pad
                                switch(m_lTracks[t].m_tTool.iType)
                                {
                                case CIRCLE:
                                {
                                    contour_t tLineCap;
                                    tLineCap.m_iType = CONTOUR_CIRCLE;
                                    tLineCap.m_lPoints.append(lLine[0] + QPointF(m_lTracks[t].m_tTool.s1 * fScale,0)); //start
                                    tLineCap.m_lPoints.append(lLine[0]); //center
                                    tLineCap.m_lPoints.append(QPointF(360,360)); //angle
                                    lContours.append(tLineCap);
                                    break;
                                }
                                case OBROUND:
                                {
                                    contour_t tLineCap;
                                    tLineCap.m_iType = CONTOUR_CIRCLE;
                                    tLineCap.m_lPoints.append(lLine[0] + QPointF(m_lTracks[t].m_tTool.s1 * fScale,0)); //start
                                    tLineCap.m_lPoints.append(lLine[0]); //center
                                    tLineCap.m_lPoints.append(QPointF(360,360)); //angle
                                    lContours.append(tLineCap);
                                    break;
                                }
                                case RECTANGLE:
                                {
                                    contour_t tLineCap;
                                    tLineCap.m_iType = CONTOUR_RECT;
                                    tLineCap.m_lPoints.append(QPointF(m_lTracks[t].m_tTool.s1* fScale,m_lTracks[t].m_tTool.s2* fScale)); //size
                                    tLineCap.m_lPoints.append(lLine[0]); //center
                                    tLineCap.m_lPoints.append(QPointF(360,360)); //angle
                                    lContours.append(tLineCap);
                                    break;
                                }

                                }
                                continue;
                            }
                            QPointF tLineVectorNormlized = Normalize(tLineVector);
                            QPointF tLinePerpendecular = Rotate90(tLineVectorNormlized);
                            QPointF tOffset;
                            switch(m_lTracks[t].m_tTool.iType)
                            {
                            case CIRCLE:
                                tOffset = tLinePerpendecular * m_lTracks[t].m_tTool.s1* fScale;
                                break;
                            default:
                                tOffset = QPointF(0,0);
                            }
                            if(isnan(tOffset.x()) || isnan(tOffset.y()))
                                throw;
                            lContour.m_lPoints.append(lLine[j - 1] +tOffset );
                            lContour.m_lPoints.append(lLine[j] + tOffset );

                            if(j == 1)
                            {
                                tLineCapStart = lLine[j-1] - tOffset;
                                bAddLineCap = true;
                            }
                        }
                        //Add line cap
                        if(bAddLineCap)
                        {
                            contour_t tLineCap;
                            tLineCap.m_iType = CONTOUR_CIRCLE;
                            tLineCap.m_lPoints.append(tLineCapStart); //start
                            tLineCap.m_lPoints.append(lLine[0]); //center
                            tLineCap.m_lPoints.append(QPointF(-180,-180)); //angle
                            lContours.append(tLineCap);
                        }
                        //Add line
                        lContours.append(lContour);


                        //Contour backward
                        bAddLineCap = false;
                        lContour.m_iType = CONTOUR_LINE;
                        lContour.m_lPoints.clear();
                        for(int  j = lLine.count()-2; j >= 0 ; j--)
                        {
                            QPointF tLineVector = lLine[j] - lLine[j + 1];
                            if(Length(tLineVector) == 0)
                                continue;
                            QPointF tLineVectorNormlized = Normalize(tLineVector);
                            QPointF tLinePerpendecular = Rotate90(tLineVectorNormlized);
                            QPointF tOffset;
                            switch(m_lTracks[t].m_tTool.iType)
                            {
                            case CIRCLE:
                                tOffset = tLinePerpendecular * m_lTracks[t].m_tTool.s1* fScale;
                                break;
                            default:
                                tOffset = QPointF(0,0);
                            }
                            if(isnan(tOffset.x()) || isnan(tOffset.y()))
                                throw;
                            //if(m_lTracks[t].m_tTool.iType == CIRCLE && (lLine[j + 1] +tOffset == lLine[j + 1] || lLine[j ] +tOffset == lLine[j]))
                             //   throw;
                            lContour.m_lPoints.append(lLine[j + 1] +tOffset );
                            lContour.m_lPoints.append(lLine[j] + tOffset );

                            if(j == lLine.count()-2)
                            {
                                tLineCapStart = lLine[j+1] - tOffset;
                                bAddLineCap = true;
                            }
                        }

                        //Add line cap
                        if(bAddLineCap)
                        {
                            contour_t tLineCap;
                            tLineCap.m_iType = CONTOUR_CIRCLE;
                            tLineCap.m_lPoints.append(tLineCapStart); //start
                            tLineCap.m_lPoints.append(lLine[lLine.count()-1]); //center
                            tLineCap.m_lPoints.append(QPointF(-180,-180)); //angle
                            lContours.append(tLineCap);
                        }
                        //Add line
                        lContours.append(lContour);

                        //DrawContours
                        for( int k =0; k < lContours.size(); k++)
                        {
                            const auto & lContour = lContours[k];
                            if(lContour.m_lPoints.count() == 0)
                            {
                                //TODO:  Draw single pad
                            }
                            else
                            {
                                if(lContour.m_iType == CONTOUR_LINE)
                                {
                                    QString sLastPosition = PixelToHpgl(lContour.m_lPoints.first());
                                    m_lGcode << ("PA" + sLastPosition);
                                    m_lGcode << "PD";
                                    for(int  j = 1; j < lContour.m_lPoints.count(); j++)
                                    {
                                        QString sNextPosition = PixelToHpgl(lContour.m_lPoints[j]);
                                        if(sNextPosition != sLastPosition)
                                        {
                                            m_lGcode << ("PA"+sNextPosition);
                                            sLastPosition = sNextPosition;
                                        }
                                    }
                                    m_lGcode << "PU";
                                }
                                else if(lContour.m_iType == CONTOUR_CIRCLE)
                                {
                                    m_lGcode << ("PA" + PixelToHpgl(lContour.m_lPoints.first()));
                                    m_lGcode << "PD";
                                    m_lGcode << ("AA" + PixelToHpgl(lContour.m_lPoints[1]) +","+ QString::number(lContour.m_lPoints[2].x()));
                                    m_lGcode << "PU";
                                }
                                else if(lContour.m_iType == CONTOUR_RECT)
                                {
                                    QPointF tSize = lContour.m_lPoints[0];
                                    QPointF tCenter = lContour.m_lPoints[1];
                                    m_lGcode << ("PA" + PixelToHpgl(tCenter + QPointF(tSize.x(),tSize.y())));
                                    m_lGcode << "PD";
                                    m_lGcode << ("PA" + PixelToHpgl(tCenter + QPointF(tSize.x(),-tSize.y())));
                                    m_lGcode << ("PA" + PixelToHpgl(tCenter + QPointF(-tSize.x(),-tSize.y())));
                                    m_lGcode << ("PA" + PixelToHpgl(tCenter + QPointF(-tSize.x(),tSize.y())));
                                    m_lGcode << ("PA" + PixelToHpgl(tCenter + QPointF(tSize.x(),tSize.y())));
                                    m_lGcode << "PU";
                                }
                            }
                        }
                    }
                }
                else//Skeleton
                {

                    for(int i =0; i <= iNumScales; i++)
                    {
                        float fScale = fStepSale * i;

                        QString sLastPosition = PixelToHpgl(Scale(lLine.first(), tCenter, fScale));
                        m_lGcode << ("PA" + sLastPosition);
                        m_lGcode << "PD";
                        for(int  j = 1; j < lLine.count(); j++)
                        {
                            QString sNextPosition = PixelToHpgl(Scale(lLine[j], tCenter, fScale));
                            if(sNextPosition != sLastPosition)
                            {
                                m_lGcode << ("PA"+sNextPosition);
                                sLastPosition = sNextPosition;
                            }
                        }
                        m_lGcode << "PU";
                    }
                }
            }
        }
    }
}

void grb2vectors::run()
{
    QStringList lLines;
    QFile tGrbFile(m_sImgFileName);
    if(tGrbFile.open(QFile::ReadOnly))
    {
        lLines = QString::fromUtf8(tGrbFile.readAll()).split("\n");
        tGrbFile.close();
    }

    QVector<QPointF> line;
    track * lCurrentTrack = nullptr;
    for(int i = 0; i < lLines.count(); i++)
    {
        QString sLine = lLines[i].toUpper();
        if(sLine.startsWith("%AD"))
        {
            QRegExp separator("(C|R|O|P|X|,)");
            QStringList lTokens = sLine.replace("%ADD","").replace("*%","").split(separator,QString::SkipEmptyParts);
            tool tTool;
            if(sLine.contains("C"))
            {
                tTool.iType = CIRCLE;
                tTool.s1 = lTokens[1].toFloat() * 15.f;
            }
            else if(sLine.contains("R"))
            {
                tTool.iType = RECTANGLE;
                tTool.s1 = lTokens[1].toFloat()* 15.f;
                tTool.s2 = lTokens[2].toFloat()* 15.f;
            }
            else if(sLine.contains("O"))
            {
                tTool.iType = OBROUND;
                tTool.s1 = lTokens[1].toFloat()* 15.f;
                tTool.s2 = lTokens[2].toFloat()* 15.f;
            }
            else if(sLine.contains("P"))
                tTool.iType = POLYGON;
            int iTrack = lTokens[0].toInt();
            m_lTools[iTrack] = tTool;
        }
        else if(sLine.startsWith("D"))
        {
            //if(line.count())
            //    m_lLines.append(line);
            //line.clear();
            if(lCurrentTrack)
                lCurrentTrack->CommitLine();
            m_lTracks.append(track());
            lCurrentTrack = &m_lTracks.back();
            lCurrentTrack->m_bFill = false;
            int iTrack = sLine.replace("D","").replace("*","").toInt();
            lCurrentTrack->m_tTool = m_lTools[iTrack];
        }
        else if(sLine.startsWith("G36"))
        {
            if(lCurrentTrack)
                lCurrentTrack->m_bFill = true;
        }
        else if(sLine.startsWith("X"))
        {
            QRegExp separator("(Y|D)");
            QStringList lCoords = sLine.remove("X").remove("*").split(separator);
            int iPlotterState = lCoords[2].toInt();
            double x = lCoords[0].toLongLong() / 100000.0 * 3.0;
            double y = lCoords[1].toLongLong() / 100000.0 * 3.0;
            if(iPlotterState == 2)
            {
                 if(lCurrentTrack)
                    lCurrentTrack->CommitLine();
                 lCurrentTrack->MoveTo(QPointF(x,y));
            }
            else if(iPlotterState == 1)
            {
                lCurrentTrack->LineTo(QPointF(x,y));
            }
            else if(iPlotterState == 3)
            {
                if(lCurrentTrack)
                    lCurrentTrack->CommitLine();
                lCurrentTrack->MoveTo(QPointF(x,y));
                lCurrentTrack->LineTo(QPointF(x,y));
            }
        }
    }
    if(lCurrentTrack)
        lCurrentTrack->CommitLine();

    InitializePrint();
    EmitLines();
    FinilizePrint();
    WriteGcode();

    qDebug() << "Done!";
    emit finished();
}
