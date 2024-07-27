#pragma once
#include <Message/Message.pb.h>

class CachedProtoData {
public:
//    static CachedProtoData& getInstance() {
//        static CachedProtoData instance;
//        return instance;
//    }
    CachedProtoData();
    proto::UserMetaInfoV1 CachedUserMetaInfo;
    bool OnlineStatus = false;

    std::string UserName;
    std::string UserToken;
    std::string Password;

};
