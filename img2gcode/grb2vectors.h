#ifndef GRB2VECTORS_H
#define GRB2VECTORS_H
#include "basetask.h"

#include <QPointF>
#include <QMap>

enum TOOL_TYPE
{
    CIRCLE = 0,
    RECTANGLE,
    OBROUND,
    POLYGON
};

struct tool
{
    TOOL_TYPE iType;
    float s1,s2,s3;
};

struct track
{
    bool m_bFill;
    tool m_tTool;
    QPointF tCurrentPos;
    QVector<QPointF>    m_lCurrentLine;
    QList<QVector<QPointF>> m_lLines;
    void CommitLine()
    {
        if(m_lCurrentLine.count())
            m_lLines.append(m_lCurrentLine);
        m_lCurrentLine.clear();
    }
    void MoveTo(const QPointF & tPoint)
    {
        tCurrentPos = tPoint;
    }
    void LineTo(const QPointF & tPoint)
    {
        if(m_lCurrentLine.isEmpty())
            m_lCurrentLine.append(tCurrentPos);
        m_lCurrentLine.append(tPoint);
        tCurrentPos = tPoint;
    }
};

class grb2vectors : public BaseTask
{
    Q_OBJECT

    QList<track>    m_lTracks;
    QMap<int,tool>  m_lTools;

    void EmitLines();
public:
    grb2vectors(const QStringList &lArgs, QObject *parent);
    ~grb2vectors();

signals:

public slots:
    void run();
};

#endif // GRB2VECTORS_H
