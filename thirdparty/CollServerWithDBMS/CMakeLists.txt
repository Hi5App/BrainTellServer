cmake_minimum_required(VERSION 3.26)
project(CollServerWithDBMS)


set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#引入 vcpkg
set(CMAKE_TOOLCHAIN_FILE "/home/seu/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Network
    Core5Compat
    REQUIRED)

find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include_directories(
    include
    include/hiredis
    include/qjson
    include/cxxopts
    include/httplib
    include/nlohmann_json
    include/octree
    include/SwcDbmsCommon/Generated/cpp
)

link_directories(
    lib
)

set(QRESOURCE

)

set(PROJECT_HEADERS
    analyze.h 
    basic_c_fun/basic_surf_objs.h 
    basic_c_fun/c_array_struct.hpp 
    basic_c_fun/color_xyz.h 
    basic_c_fun/v3d_basicdatatype.h 
    basic_c_fun/v3d_message.h 
    basic_c_fun/v_neuronswc.h 
    coll_server.h 
    collclient.h 
    colldetection.h 
    collthread.h 
    neuron_editing/apo_xforms.h 
    neuron_editing/global_feature_compute.h 
    neuron_editing/neuron_format_converter.h 
    neuron_editing/neuron_xforms.h 
    neuron_editing/v_neuronswc.h 
    sort_swc.h 
    utils.h
    ./include/hiredis/hiredis.h
    include/cxxopts/cxxopts.hpp
    include/httplib/httplib.h
    include/nlohmann_json/json.hpp
    include/octree/octree.h
    include/octree/octree_container.h
    detect_crossing/ConfigManager.h
    detect_crossing/CrossingDetect.h
    detect_crossing/utilities.h
    detect_crossing/SwcReader.h
    detect_crossing/ResultWriter.h
    config/config.h
)

set(PROJECT_SOURCES
    analyze.cpp
    basic_c_fun/basic_surf_objs.cpp
    basic_c_fun/v3d_message.cpp
    coll_server.cpp
    collclient.cpp
    colldetection.cpp
    collthread.cpp
    main.cpp
    neuron_editing/apo_xforms.cpp
    neuron_editing/global_feature_compute.cpp
    neuron_editing/neuron_format_converter.cpp
    neuron_editing/neuron_xforms.cpp
    neuron_editing/v_neuronswc.cpp
    sort_swc.cpp
    utils.cpp
    detect_crossing/ConfigManager.cpp
    detect_crossing/CrossingDetect.cpp
    detect_crossing/ResultWriter.cpp
    config/config.cpp
    include/SwcDbmsCommon/Generated/cpp/Message/Message.grpc.pb.cc
    include/SwcDbmsCommon/Generated/cpp/Message/Message.pb.cc
    include/SwcDbmsCommon/Generated/cpp/Message/Request.grpc.pb.cc
    include/SwcDbmsCommon/Generated/cpp/Message/Request.pb.cc
    include/SwcDbmsCommon/Generated/cpp/Message/Response.grpc.pb.cc
    include/SwcDbmsCommon/Generated/cpp/Message/Response.pb.cc
    include/SwcDbmsCommon/Generated/cpp/Service/Service.grpc.pb.cc
    include/SwcDbmsCommon/Generated/cpp/Service/Service.pb.cc
    service/RpcCall.cpp
    service/RpcCall.h
    service/CachedProtoData.cpp
    service/CachedProtoData.h
    service/WrappedCall.cpp
    service/WrappedCall.h
    FileIo/AnoIo.hpp
    FileIo/ApoIo.hpp
    FileIo/SwcIo.hpp
    FileIo/FileIoInterface.hpp
)

add_executable(${PROJECT_NAME} ${QRESOURCE} ${PROJECT_HEADERS} ${PROJECT_SOURCES})

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Network
    Qt::Core5Compat
    libhiredis.so
    pthread
    PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
    PRIVATE spdlog::spdlog
    PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Network
    Qt::Core5Compat
    libhiredis.a
    pthread
    PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
    PRIVATE spdlog::spdlog
    PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts
    )
endif(UNIX)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Network
    Qt::Core5Compat
    pthread
    )
endif(WIN32)

# if (WIN32)
#     Set_Target_Properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
# endif ()
