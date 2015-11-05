#-------------------------------------------------
#
# Project created by QtCreator 2015-08-13T09:21:34
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fea_gui
TEMPLATE = app

EXT_BOOST_ROOT = ../ext/boost_1_59_0
EXT_EIGEN_ROOT = ../ext/eigen-3.2.4
EXT_RAPIDJSON_ROOT = ../ext/rapidjson/include
FEA_INCLUDE_ROOT = ../include
FEA_SRC_ROOT = ../src

ICON = images/logo_64x64.png

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += $${EXT_BOOST_ROOT} \
               $${EXT_EIGEN_ROOT} \
               $${EXT_RAPIDJSON_ROOT} \
               $${FEA_INCLUDE_ROOT}

SOURCES += main.cpp \
           mainwindow.cpp \
           $${EXT_BOOST_ROOT}/libs/smart_ptr/src/sp_collector.cpp \
           $${EXT_BOOST_ROOT}/libs/smart_ptr/src/sp_debug_hooks.cpp \
           $${FEA_SRC_ROOT}/threed_beam_fea.cpp \
           $${FEA_SRC_ROOT}/summary.cpp \
           $${FEA_SRC_ROOT}/setup.cpp

HEADERS  += mainwindow.h \
           $${FEA_INCLUDE_ROOT}/threed_beam_fea.h \
           $${FEA_INCLUDE_ROOT}/summary.h \
           $${FEA_INCLUDE_ROOT}/setup.h \
           $${FEA_INCLUDE_ROOT}/containers.h \
           $${FEA_INCLUDE_ROOT}/csv_parser.h \
           $${FEA_INCLUDE_ROOT}/options.h

RESOURCES += fea_gui.qrc
