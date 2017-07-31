#ifndef IMG2VECTORS_H
#define IMG2VECTORS_H

#include "basetask.h"

class img2vectors : public BaseTask
{
    Q_OBJECT
public:
    explicit img2vectors(const QStringList &lArgs, QObject *parent = nullptr);

signals:

public slots:
    void run();
};

#endif // IMG2VECTORS_H
