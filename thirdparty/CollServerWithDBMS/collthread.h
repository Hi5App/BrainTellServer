#ifndef COLLTHREAD_H
#define COLLTHREAD_H

#include <QObject>
#include <QThread>

class CollServer;
class CollThread : public QThread
{
    Q_OBJECT
public:
    explicit CollThread(QObject* parent=nullptr);
    ~CollThread() override;
//    CollServer* myServer;
    CollServer* myServer;

    void run() override;
    void setServer(CollServer* curServer);

};

#endif // COLLTHREAD_H
