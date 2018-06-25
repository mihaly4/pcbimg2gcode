#ifndef IMG2VECTORS_H
#define IMG2VECTORS_H
#include <QPoint>
#include <QVector>
#include "basetask.h"
#include <opencv2/imgproc/imgproc.hpp>
struct contour_t
{
    std::vector<std::vector<cv::Point> > m_lContours;
    std::vector<cv::Vec4i> m_lHierarchy;

    contour_t()
    {

    }

    contour_t(const contour_t & other)
    {
        m_lContours = other.m_lContours;
        m_lHierarchy = other.m_lHierarchy;
    }

    void Draw(cv::Mat & img_draw, const cv::Scalar & color) const
    {
        for( int idx2 = 0 ; idx2 >= 0; idx2 = m_lHierarchy[idx2][0] )
        {
            cv::drawContours(img_draw, m_lContours, idx2, color, 1 /*CV_FILLED*/, 8, m_lHierarchy, 1);
            //num_inside++;
        }
    }

    int LineCount() const
    {
        int count = 0;
        for( int idx2 = 0 ; idx2 >= 0; idx2 = m_lHierarchy[idx2][0] )
        {
            count++;
        }
        return count;
    }

    QList<QVector<QPoint>> GetLines() const
    {
        QList<QVector<QPoint>> lines;
        //for( int idx2 = 0 ; idx2 >= 0; idx2 = m_lHierarchy[idx2][0] )
        for(int idx2 = 0; idx2 <m_lContours.size(); idx2++ )
        {
            QVector<QPoint> line;
            const std::vector<cv::Point>  & contour = m_lContours[idx2];
            for(int p = 0; p < contour.size(); p++)
            {
                line.append(QPoint(contour[p].x, contour[p].y));
                //line.append(QPoint(contour[p+1].x, contour[p+1].y));
            }
            //line.append(QPoint(contour.back().x, contour.back().y));
            line.append(QPoint(contour.front().x, contour.front().y));
            lines.append(line);
        }
        return lines;
    }
};

struct track_t
{
    QList<contour_t>    m_lContours;
    cv::Scalar          m_tColor;
    int                 m_iHindex;

    track_t(int idx)
    {
        m_iHindex = idx;
        m_tColor = cv::Scalar(rand() & 255, rand() & 255, rand() & 255);
    }

    track_t(const track_t & other)
    {
        m_lContours = other.m_lContours;
        m_tColor = other.m_tColor;
        m_iHindex = other.m_iHindex;
    }

    void Draw(cv::Mat & img_draw) const
    {
        foreach(auto & contour,m_lContours)
        {
            contour.Draw(img_draw, m_tColor);
        }
       // cv::drawContours(img_draw, THIS->tracks_contours, track.m_iHindex, track.m_tColor, CV_FILLED, 8, THIS->tracks_hierarchy, 1, cv::Point());
    }

    int LineCount()
    {
        int res = 0;
        foreach(auto & contour,m_lContours)
        {
            res += contour.LineCount();
        }
        return res;
    }

    QList<QVector<QPoint>> GetLines() const
    {
        QList<QVector<QPoint>> lines;
        foreach(auto & contour,m_lContours)
        {
            lines.append(contour.GetLines());
        }
        return lines;
    }
};

class img2vectors : public BaseTask
{
    Q_OBJECT
    cv::Mat src_gray;
    cv::Mat drawing;
    cv::Mat src;

    std::vector<std::vector<cv::Point> >    tracks_contours;
    std::vector<cv::Vec4i>                  tracks_hierarchy;
    QList<track_t>                          m_lTracks;
    QList<QVector<QPoint>>                  m_lLines;

    void CreateTracks();
    void CreateSkeleton(const cv::Mat &track_img, track_t &track);
    contour_t ExtractContours(const cv::Mat &src_img);
    static void thresh2_callback(int, void *data);
    void DrawResult();
    void EmitLines();
    QString PixelToHpgl(const QPoint & pt);
public:
    explicit img2vectors(const QStringList &lArgs, QObject *parent = nullptr);

signals:

public slots:
    void run();
};

#endif // IMG2VECTORS_H
