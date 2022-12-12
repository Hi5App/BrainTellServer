#include "coll_server.h"
#include <unistd.h>
#include "utils.h"
extern QFile *logfile;
CollServer::CollServer(QString port,QString image,QString neuron,QString anoname,QString prefix,QObject *parent)
    :QTcpServer(parent),Port(port),Image(image),Neuron(neuron),AnoName(anoname),Prefix(prefix+"/data/"+image+"/"+neuron+"/"+anoname)
    ,timerForAutoSave(new QTimer(this))
{
    auto nt=readSWC_file(Prefix+"/"+AnoName+".ano.eswc");
    CollClient::segments=NeuronTree__2__V_NeuronSWC_list(nt);
    CollClient::markers=readAPO_file(Prefix+"/"+AnoName+".ano.apo");
    CollClient::swcpath=Prefix+"/"+AnoName+".ano.eswc";
    CollClient::apopath=Prefix+"/"+AnoName+".ano.apo";
    CollClient::anopath=Prefix+"/"+AnoName+".ano";
    timerForAutoSave->start(3*60*1000);
    CollClient::timerforupdatemsg.start(1000);
    CollClient::msglist.reserve(5000);

    if(!this->listen(QHostAddress::Any,Port.toInt())){
        std::cerr<<"Can not init server with port "<<Port.toInt()<<std::endl;
        exit(-1);
    }
    connect(timerForAutoSave,&QTimer::timeout,this,&CollServer::autosave);
    connect(&CollClient::timerforupdatemsg,&QTimer::timeout,[]{
        auto sockets=CollClient::hashmap.values();
        for(auto &socket:sockets){
            socket->sendmsgs2client(10);
        }
    });
    setredis(Port.toInt(),anoname.toStdString().c_str());
}

CollServer::~CollServer(){
    // change set expire time 60 -> 10
    setexpire(Port.toInt(),AnoName.toStdString().c_str(),10);
    // recover port
    recoverPort(Port.toInt());
    std::cerr<<AnoName.toStdString()+" server is released\n";
    logfile->flush();
    exit(0);
}

void CollServer::incomingConnection(qintptr handle){
    new CollClient(handle,this);
}

void CollServer::autosave()
{
    std::cout<<"auto save\n"<<std::endl;
    logfile->flush();
    fsync(1);fsync(2);
     if(CollClient::hashmap.size()==0){
        //没有用户了
        this->close();
        writeESWC_file(Prefix+"/"+AnoName+".ano.eswc",V_NeuronSWC_list__2__NeuronTree(CollClient::segments));
        writeAPO_file(Prefix+"/"+AnoName+".ano.apo",CollClient::markers);
        deleteLater();
    }else{ 
        auto sockets=CollClient::hashmap.values();
        for(auto &socket:sockets)
            socket->updatesendmsgcnt2processed();
        CollClient::msglist.erase(CollClient::msglist.begin(),CollClient::msglist.begin()+CollClient::processedmsgcnt);
        CollClient::msglist.reserve(5000);
        CollClient::savedmsgcnt+=CollClient::processedmsgcnt;
        CollClient::processedmsgcnt=0;

        writeESWC_file(Prefix+"/"+AnoName+".ano.eswc",V_NeuronSWC_list__2__NeuronTree(CollClient::segments));
        writeAPO_file(Prefix+"/"+AnoName+".ano.apo",CollClient::markers);

    }
}

