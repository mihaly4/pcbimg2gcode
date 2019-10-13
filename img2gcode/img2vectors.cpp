#include "img2vectors.h"
#include <QDebug>
#include <vector>
#include <QImage>
#include <QThread>
#include <QCoreApplication>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int erose = 3;

cv::RNG rng(12345);


void img2vectors::CreateTracks()
{
    cv::findContours(src_gray, tracks_contours, tracks_hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);//CV_CHAIN_APPROX_TC89_L1, cv::Point(0, 0));
    for(int idx = 0 ; idx >= 0; idx = tracks_hierarchy[idx][0] )
    {
        track_t track(idx);
        m_lTracks.append(track);
    }
    qDebug() << "Found "<<m_lTracks.count() << " tracks";
}

void img2vectors::CreateSkeleton(const cv::Mat & track_img, track_t & track)
{
    int size = track_img.total();
    cv::Mat skel = cv::Mat::zeros(track_img.size(), CV_8UC1);
    cv::Mat img_thresholded = track_img.clone();
    cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3));
    bool done = false;

    while( ! done)
    {
        cv::Mat eroded;
        cv::erode(img_thresholded, eroded, element);
        cv::Mat temp;
        cv::dilate(eroded, temp, element);
        cv::subtract(img_thresholded,temp, temp);

        cv::bitwise_or(skel,temp, skel);
        img_thresholded = eroded.clone();

        int zeros = size - cv::countNonZero(img_thresholded);
        if (zeros==size)
            done = true;
    }
    track.m_lContours.append(ExtractContours(skel));
}

contour_t img2vectors::ExtractContours(const cv::Mat &src_img)
{
    contour_t result;
    cv::findContours(src_img, result.m_lContours, result.m_lHierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    return result;
}

img2vectors::img2vectors(const QStringList & lArgs, QObject *parent) : BaseTask(lArgs, parent)
{

}

void img2vectors::thresh2_callback(int, void* data)
{
    img2vectors * THIS= (img2vectors*)data;
    THIS->m_lTracks.clear();
    THIS->m_lLines.clear();
    THIS->CreateTracks();
    cv::Mat img_track = cv::Mat::zeros(THIS->src_gray.size(), CV_8UC1);
    for(int i =0; i < THIS->m_lTracks.count(); i++)
    {
        track_t & track = THIS->m_lTracks[i];

        //Prepare track image
        img_track.setTo(cv::Scalar(0));
        cv::drawContours(img_track, THIS->tracks_contours, track.m_iHindex, cv::Scalar(255), cv::FILLED, 8, THIS->tracks_hierarchy, 1, cv::Point());

        //Edore and create contours

        int w = std::max(erose, 1);
        int erosion_elem = 2;
        int erosion_type = 0;
        if( erosion_elem == 0 ){ erosion_type = cv::MORPH_RECT; }
        else if( erosion_elem == 1 ){ erosion_type = cv::MORPH_CROSS; }
        else if( erosion_elem == 2) { erosion_type = cv::MORPH_ELLIPSE; }
        cv::Mat element = cv::getStructuringElement( erosion_type, cv::Size( 2*w + 1, 2*w+1 ), cv::Point( w, w ) );
        //cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(w,w));
        cv::Mat lastNonEmptyContour;// = img_track.clone();
        while(true)
        {

            contour_t tContour;
            cv::findContours(img_track, tContour.m_lContours, tContour.m_lHierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);//CV_CHAIN_APPROX_TC89_L1, cv::Point(0, 0));
            if(tContour.m_lHierarchy.size() == 0)
                break;
            if(tContour.LineCount() == 0)
                break;
            track.m_lContours.append(tContour);
            if(lastNonEmptyContour.total() == 0)
                lastNonEmptyContour = img_track.clone();

            /// Draw rect arount everything, cuz erode does not work on image bounds
            cv::rectangle(img_track, cv::Rect(0,0, img_track.cols -1, img_track.rows -1), cv::Scalar(0));
            /// Apply the erosion operation
            cv::erode( img_track, img_track, element );
        }

        //Find skeleton
        if(lastNonEmptyContour.total() > 0)
            THIS->CreateSkeleton(lastNonEmptyContour, track);

        THIS->m_lLines.append(track.GetLines());
        qDebug() << "Track "<<(i+1) <<" finished. Got " << track.LineCount();


    }

    //THIS->RemoveCircles();
    THIS->SimplifyDiagonals();
    THIS->DrawResult();
    THIS->InitializePrint();
    THIS->EmitLines();
    THIS->FinilizePrint();
    THIS->WriteGcode();

}

void img2vectors::DrawResult()
{
    drawing = cv::Mat::zeros(src_gray.size(), CV_8UC3);
    for(int i = 0; i < m_lTracks.size(); i++)
    {
        m_lTracks[i].Draw(drawing);
        //cv::imshow("Contours", drawing);
        //qApp->processEvents();
        //QThread::currentThread()->msleep(500);
    }
    cv::imshow("Contours", drawing);
}

void img2vectors::EmitLines()
{
    for(int i = 0; i < m_lLines.count(); i++)
    {
        const QVector<QPoint> & lLine = m_lLines[i];
        if(lLine.size())
        {

            QString sLastPosition = PixelToHpgl(lLine.first());
            m_lGcode << ("PA" + sLastPosition);
            m_lGcode << "PD";
            for(int  j = 1; j < lLine.count(); j++)
            {
                QString sNextPosition = PixelToHpgl(lLine[j]);
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

float Distance2Line(QPoint lineP1, QPoint lineP2, QPoint testPoint)
{
    float a = lineP1.y() -lineP2.y();
    float b = lineP2.x() -lineP1.x();
    float c = (lineP1.x() - lineP2.x()) * lineP1.y() + (lineP2.y() - lineP1.y()) * lineP1.x();
    return abs(a*testPoint.x() + b *testPoint.y() + c) / sqrtf(a*a + b*b);
}

void img2vectors::SimplifyDiagonals()
{
    for(int i = 0; i < m_lLines.count(); i++)
    {
        QVector<QPoint> lLine = m_lLines[i];
        if(lLine.size())
        {

            int iStartIndex = 0;
            int tLastValid = 1;



            for(int  j = 2; j < lLine.count(); j++)
            {

                //float fDistance = Distance2Line(lLine[iStartIndex], lLine[iStartIndex + (j - iStartIndex) / 2], lLine[j]);
                float fDistance = 0;
                for(int t = iStartIndex + 1; t < j; t++)
                {
                    fDistance = std::max(fDistance, Distance2Line(lLine[iStartIndex], lLine[j], lLine[t]));
                }
                if(fDistance >= 0.5)
                {
                    int iNumPointToRemove = tLastValid - iStartIndex - 1;
                    if(iNumPointToRemove > 0)
                    {
                        for(int del = 0; del < iNumPointToRemove; del++)
                        {
                            lLine.removeAt(iStartIndex + 1);
                            j--;
                        }
                        qDebug() << iNumPointToRemove << " diagonal points removed";
                    }
                    iStartIndex = j - 1;
                    tLastValid = j;

                }
                else
                {
                    tLastValid = j;
                }

            }

            int iNumPointToRemove = tLastValid - iStartIndex - 1;
            if(iNumPointToRemove > 0)
            {
                for(int del = 0; del < iNumPointToRemove; del++)
                {
                    lLine.removeAt(iStartIndex + 1);
                }
                qDebug() << iNumPointToRemove << " diagonal points removed";
            }

            m_lLines[i] = lLine;
        }
    }
}

float angleBwPoints(const QPoint &V1, const QPoint &V2)
{
    float length1 = sqrtf(V1.x() * V1.x() + V1.y() * V1.y());// calculate modulus of Vector V1 i.e. |V1|
    float length2 = sqrtf(V2.x() * V2.x() + V2.y() * V2.y());
// calculate modulus of Vector V2 i.e. |V2|
    float dot = V1.x() * V2.x() + V1.y() * V2.y(); // calculate dot product between two vectors.

    float a = dot / (length1 * length2);

    if (a >= 1.0)
        return 0.0;
    else if (a <= -1.0)
        return M_PI;// PI means π
    else
        return acos(a); // 0..PI
}

void img2vectors::RemoveCircles()
{
    for(int i = 0; i < m_lLines.count(); i++)
    {
        QVector<QPoint> lLine = m_lLines[i];
        if(lLine.size())
        {

            QPoint tStart = lLine.first();



            for(int  j = 1; j < lLine.count()-1; j++)
            {
                QPoint tTest = lLine[j];

                QPoint tDist1 = tTest - tStart;
                QPoint tDist2 = tTest - lLine[j+1];

                float fDistance1 = sqrtf(tDist1.x() * tDist1.x() + tDist1.y() * tDist1.y());
                float fDistance2 = sqrtf(tDist2.x() * tDist2.x() + tDist2.y() * tDist2.y());
                float fMaxDistance = std::max(fDistance1,fDistance2);
                float fMinDistance = std::min(fDistance1,fDistance2);

                float angle = angleBwPoints(tStart - tTest, lLine[j+1] - tTest);
                /*if(abs(angle - M_PI) < M_PI / 10.0f)//*/if(fDistance1 < 12 && fDistance2 < 12)
                {
                    lLine.remove(j);
                    j--;
                }
                tStart = tTest;


            }


            m_lLines[i] = lLine;
        }
    }
}



void img2vectors::run()
{



    src = cv::Mat(m_pSrcImage->height(), m_pSrcImage->width(), CV_8UC4, m_pSrcImage->scanLine(0));

    /// Convert image to bw
    cv::cvtColor( src, src_gray, cv::COLOR_BGR2GRAY );
    cv::threshold( src_gray, src_gray, 20, 255, 0 );
    drawing = cv::Mat::zeros(src_gray.size(), CV_8UC3);

    //cv::namedWindow( "Source", CV_WINDOW_AUTOSIZE );
    cv::namedWindow("Contours", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("Skeleton", CV_WINDOW_AUTOSIZE);
    cv::imshow( "Source", src );



    cv::createTrackbar( " Erode:", "Source", &erose, 10, thresh2_callback, this);

    thresh2_callback(0,this);
    cv::waitKey(0);





    qDebug() << "Done!";
    emit finished();
}
