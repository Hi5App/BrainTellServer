#pragma once

#include <QString>
#include "grpcpp/client_context.h"
#include "Message/Request.pb.h"
#include "RpcCall.h"
#include "CachedProtoData.h"
#include "qdebug.h"

class WrappedCall {
public:
    template<typename T>
    static void setCommonRequestField(T&type, CachedProtoData& cachedUserData) {
        type.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
        auto* userInfo = type.mutable_userverifyinfo();
        userInfo->set_username(cachedUserData.UserName);
        userInfo->set_userpassword(cachedUserData.Password);
    }

    static bool defaultErrorHandler(const std::string&actionName, const grpc::Status&status,
                                    const proto::ResponseMetaInfoV1&rspMeta) {
        if (status.ok()) {
            if (rspMeta.status()) {
                return true;
            }
            qDebug()<<QString::fromStdString(actionName + " Failed!" + rspMeta.message());
        }
        qDebug()<<QString::fromStdString(status.error_message());
        return false;
    }

    static bool getAllProjectMetaInfo(proto::GetAllProjectResponse&response, CachedProtoData& cachedUserData) {
        grpc::ClientContext context;
        proto::GetAllProjectRequest request;
        setCommonRequestField(request, cachedUserData);
        auto status = RpcCall::getInstance().Stub()->GetAllProject(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool getSwcMetaInfoByName(const std::string&swcName, proto::GetSwcMetaInfoResponse&response,
                                    CachedProtoData& cachedUserData) {
        proto::GetSwcMetaInfoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(swcName);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->GetSwcMetaInfo(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool getSwcFullNodeData(const std::string&swcName, proto::GetSwcFullNodeDataResponse&response,
                                   CachedProtoData& cachedUserData) {
        proto::GetSwcFullNodeDataRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(swcName);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->GetSwcFullNodeData(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool addSwcNodeData(const std::string&swcName, proto::SwcDataV1&swcData,
                               proto::CreateSwcNodeDataResponse&response, CachedProtoData& cachedUserData) {
        proto::CreateSwcNodeDataRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(swcName);
        request.mutable_swcdata()->CopyFrom(swcData);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->CreateSwcNodeData(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool modifySwcNodeData(const std::string&swcName, proto::SwcDataV1&swcData,
                                  proto::UpdateSwcNodeDataResponse&response, CachedProtoData& cachedUserData) {
        proto::UpdateSwcNodeDataRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(swcName);
        request.mutable_swcdata()->CopyFrom(swcData);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->UpdateSwcNodeData(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool deleteSwcNodeData(const std::string&swcName, proto::SwcDataV1&swcData,
                                  proto::DeleteSwcNodeDataResponse&response, CachedProtoData& cachedUserData) {
        proto::DeleteSwcNodeDataRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(swcName);
        request.mutable_swcdata()->CopyFrom(swcData);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->DeleteSwcNodeData(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool createSwcMeta(const std::string&name, const std::string&description, proto::CreateSwcResponse&response,
                              CachedProtoData& cachedUserData) {
        proto::CreateSwcRequest request;
        setCommonRequestField(request, cachedUserData);
        request.mutable_swcinfo()->set_name(name);
        request.mutable_swcinfo()->set_description(description);
        request.mutable_swcinfo()->set_swctype("eswc"); // by default when creating new swc using eswc type

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->CreateSwc(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool getAllSwcIncrementRecord(const std::string&name,
                                         proto::GetAllIncrementOperationMetaInfoResponse&response,
                                         CachedProtoData& cachedUserData) {
        proto::GetAllIncrementOperationMetaInfoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(name);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->GetAllIncrementOperationMetaInfo(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool getSwcIncrementRecord(const std::string&name,
                                      proto::GetIncrementOperationResponse&response,
                                      CachedProtoData& cachedUserData) {
        proto::GetIncrementOperationRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_incrementoperationcollectionname(name);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->GetIncrementOperation(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool getSwcAttachmentAno(const std::string&swcName, const std::string&attachmentUuid,
                                    proto::GetSwcAttachmentAnoResponse&response,
                                    CachedProtoData& cachedUserData) {
        proto::GetSwcAttachmentAnoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(swcName);
        request.set_anoattachmentuuid(attachmentUuid);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->GetSwcAttachmentAno(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool createSwcAttachmentAno(const std::string&name, const std::string&apoFileName,
                                       const std::string&swcFileName,
                                       proto::CreateSwcAttachmentAnoResponse&response,
                                       CachedProtoData& cachedUserData) {
        proto::CreateSwcAttachmentAnoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(name);
        request.mutable_swcattachmentano()->set_apofile(apoFileName);
        request.mutable_swcattachmentano()->set_swcfile(swcFileName);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->CreateSwcAttachmentAno(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool updateSwcAttachmentAno(const std::string&name, const std::string&attachmentUuid,
                                       const std::string&apoFileName, const std::string&swcFileName,
                                       proto::UpdateSwcAttachmentAnoResponse&response,
                                       CachedProtoData& cachedUserData) {
        proto::UpdateSwcAttachmentAnoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(name);
        request.set_anoattachmentuuid(attachmentUuid);
        request.mutable_newswcattachmentano()->set_apofile(apoFileName);
        request.mutable_newswcattachmentano()->set_swcfile(swcFileName);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->UpdateSwcAttachmentAno(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool deleteSwcAttachmentAno(const std::string&name, const std::string&attachmentUuid,
                                       proto::DeleteSwcAttachmentAnoResponse&response,
                                       CachedProtoData& cachedUserData) {
        proto::DeleteSwcAttachmentAnoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(name);
        request.set_anoattachmentuuid(attachmentUuid);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->DeleteSwcAttachmentAno(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool getSwcAttachmentApo(const std::string&swcName, const std::string&attachmentUuid,
                                    proto::GetSwcAttachmentApoResponse&response,
                                    CachedProtoData& cachedUserData) {
        proto::GetSwcAttachmentApoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(swcName);
        request.set_apoattachmentuuid(attachmentUuid);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->GetSwcAttachmentApo(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool createSwcAttachmentApo(const std::string&name, std::vector<proto::SwcAttachmentApoV1> attachments,
                                       proto::CreateSwcAttachmentApoResponse&response,
                                       CachedProtoData& cachedUserData) {
        proto::CreateSwcAttachmentApoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(name);
        for (auto&attachment: attachments) {
            request.add_swcattachmentapo()->CopyFrom(attachment);
        }

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->CreateSwcAttachmentApo(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool updateSwcAttachmentApo(const std::string&name, const std::string&attachmentUuid,
                                       std::vector<proto::SwcAttachmentApoV1> attachments,
                                       proto::UpdateSwcAttachmentApoResponse&response,
                                       CachedProtoData& cachedUserData) {
        proto::UpdateSwcAttachmentApoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(name);
        request.set_apoattachmentuuid(attachmentUuid);
        for (auto&attachment: attachments) {
            request.add_newswcattachmentapo()->CopyFrom(attachment);
        }

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->UpdateSwcAttachmentApo(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }

    static bool deleteSwcAttachmentApo(const std::string&name, const std::string&attachmentUuid,
                                       proto::DeleteSwcAttachmentApoResponse&response,
                                       CachedProtoData& cachedUserData) {
        proto::DeleteSwcAttachmentApoRequest request;
        setCommonRequestField(request, cachedUserData);
        request.set_swcname(name);
        request.set_apoattachmentuuid(attachmentUuid);

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->DeleteSwcAttachmentApo(&context, request, &response);
        return defaultErrorHandler(__func__, status, response.metainfo());
    }
};
