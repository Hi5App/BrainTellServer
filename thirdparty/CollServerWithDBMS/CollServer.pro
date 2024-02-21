QT += gui
QT += network core5compat
CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        analyze.cpp \
        basic_c_fun/basic_surf_objs.cpp \
        basic_c_fun/v3d_message.cpp \
        coll_server.cpp \
        collclient.cpp \
        colldetection.cpp \
        collthread.cpp \
        main.cpp \
        neuron_editing/apo_xforms.cpp \
        neuron_editing/global_feature_compute.cpp \
        neuron_editing/neuron_format_converter.cpp \
        neuron_editing/neuron_xforms.cpp \
        neuron_editing/v_neuronswc.cpp \
        sort_swc.cpp \
        utils.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    analyze.h \
    basic_c_fun/basic_surf_objs.h \
    basic_c_fun/c_array_struct.hpp \
    basic_c_fun/color_xyz.h \
    basic_c_fun/v3d_basicdatatype.h \
    basic_c_fun/v3d_message.h \
    basic_c_fun/v_neuronswc.h \
    coll_server.h \
    collclient.h \
    colldetection.h \
    collthread.h \
    neuron_editing/apo_xforms.h \
    neuron_editing/global_feature_compute.h \
    neuron_editing/neuron_format_converter.h \
    neuron_editing/neuron_xforms.h \
    neuron_editing/v_neuronswc.h \
    sort_swc.h \
    utils.h

HEADERS += $$PWD/include/hiredis/hiredis.h

DISTFILES +=

macx: LIBS += -L$$PWD/lib/ -lhiredis
LIBS +=-L$$PWD/lib/ -lhiredis

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

macx: PRE_TARGETDEPS += $$PWD/lib/libhiredis.a
PRE_TARGETDEPS += $$PWD/lib/libhiredis.a
