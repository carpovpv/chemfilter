#ifndef CLEARTHREAD_H
#define CLEARTHREAD_H

#include <QThread>

class ClearThread : public QThread
{
    Q_OBJECT
public:
    void run();
};

#endif // CLEARTHREAD_H
