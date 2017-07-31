#include "img2vectors.h"
#include <QDebug>
#include <vector>
#include <QImage>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

cv::Mat src; cv::Mat src_gray; cv::Mat src_erode;
int erose = 1;
cv::RNG rng(12345);
std::vector<std::vector<cv::Point> > contours;
std::vector<cv::Vec4i> hierarchy;

img2vectors::img2vectors(const QStringList & lArgs, QObject *parent) : BaseTask(lArgs, parent)
{

}

void thresh2_callback(int, void* )
{
    //for (auto vec : hierarchy)
    //  qDebug() << vec; // just to have a look at the hierarchy vector
    /*int count = 0;
    for (int i = 0; i < 5; i++)
    {
        if (hierarchy[i][3] != 0)
        {
            count++;
        }
    }
    qDebug() << "number of labels = " << count;*/

    int w = std::max(erose, 1);
    int erosion_elem = 0;
    int erosion_type;
    if( erosion_elem == 0 ){ erosion_type = cv::MORPH_RECT; }
    else if( erosion_elem == 1 ){ erosion_type = cv::MORPH_CROSS; }
    else if( erosion_elem == 2) { erosion_type = cv::MORPH_ELLIPSE; }
    cv::Mat element = cv::getStructuringElement( erosion_type, cv::Size( 2*w + 1, 2*w+1 ), cv::Point( w, w ) );

    cv::Mat drawing = cv::Mat::zeros(src_gray.size(), CV_8UC3);

    for(int idx = 0 ; idx >= 0; idx = hierarchy[idx][0] )
    {
        cv::Scalar color = cv::Scalar(rand() & 255, rand() & 255, rand() & 255);;
        cv::drawContours(drawing, contours, idx, color, 1 /*CV_FILLED*/, 8, hierarchy, 1, cv::Point());
        cv::Mat erode_src = cv::Mat::zeros(src_gray.size(), CV_8UC1);
        cv::drawContours(erode_src, contours, idx, cv::Scalar(255,255,255), CV_FILLED, 8, hierarchy, 1, cv::Point());

        int num_inside = 0;
        do
        {
            /// Apply the erosion operation
            erode( erode_src, erode_src, element );
            std::vector<std::vector<cv::Point> > contours2;
            std::vector<cv::Vec4i> hierarchy2;
            cv::findContours(erode_src, contours2, hierarchy2, cv::RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);//CV_CHAIN_APPROX_TC89_L1, cv::Point(0, 0));
            if(hierarchy2.size() == 0)
                break;
            num_inside = 0;
            for( int idx2 = 0 ; idx2 >= 0; idx2 = hierarchy2[idx2][0] )
            {
                cv::drawContours(drawing, contours2, idx2, color, 1 /*CV_FILLED*/, 8, hierarchy2, 1);
                num_inside++;
            }
        }while(num_inside);
    }
    cv::imshow("Contours", drawing);
}

void img2vectors::run()
{
    InitializePrint();


    src = cv::Mat(m_pSrcImage->height(), m_pSrcImage->width(), CV_8UC4, m_pSrcImage->scanLine(0));

    /// Convert image to bw
    cv::cvtColor( src, src_gray, CV_BGR2GRAY );
    cv::threshold( src_gray, src_gray, 20, 255, 0 );


    cv::namedWindow( "Source", CV_WINDOW_AUTOSIZE );
    cv::namedWindow("Contours", CV_WINDOW_AUTOSIZE);
    cv::imshow( "Source", src );


    cv::findContours(src_gray, contours, hierarchy, cv::RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);//CV_CHAIN_APPROX_TC89_L1, cv::Point(0, 0));
    cv::createTrackbar( " Erode:", "Source", &erose, 10, thresh2_callback );


    thresh2_callback(0,0);
    cv::waitKey(0);


    FinilizePrint();
    WriteGcode();
    qDebug() << "Done!";
    emit finished();
}
