#pragma once
#include <string>
#include <grpcpp/grpcpp.h>
#include <Service/Service.grpc.pb.h>

class RpcCall {
public:
    RpcCall(RpcCall&) = delete;
    RpcCall& operator=(RpcCall&) = delete;

    void initialize(const std::string& endpoint) {
        m_Endpoint = endpoint;
        grpc::ChannelArguments channelArgs;
        channelArgs.SetCompressionAlgorithm(GRPC_COMPRESS_GZIP);
        channelArgs.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, 256 * 1024 * 1024); // 256mb
        channelArgs.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, 256 * 1024 * 1024); // 256mb

        m_Channel = grpc::CreateCustomChannel(
                m_Endpoint, grpc::InsecureChannelCredentials(), channelArgs);

        m_Stub = proto::DBMS::NewStub(m_Channel);

    }

    static RpcCall& getInstance() {
        static RpcCall instance;
        return instance;
    }

    auto& Endpoint() {
        return m_Endpoint;
    }

    auto& Channel() {
        return m_Channel;
    }

    auto& Stub() {
        return m_Stub;
    }

    inline static std::string ApiVersion = "2024.01.19";

private:
    RpcCall() {
    }

    std::string m_Endpoint;
    std::shared_ptr<grpc::Channel> m_Channel;
    std::unique_ptr<proto::DBMS::Stub> m_Stub;
};
