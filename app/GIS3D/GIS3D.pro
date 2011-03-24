#-------------------------------------------------
#
# Project created by QtCreator 2010-11-11T18:41:08
#
#-------------------------------------------------

QT       += core gui opengl \
    sql \
    svg \
    xml
TARGET = GIS3D
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    AdapterWidget.cpp \
    osggislayer.cpp \
    dialog.cpp \
    osggistreeitem.cpp \
    filterstackdlg2.cpp

HEADERS  += mainwindow.h \
    AdapterWidget.h \
    osggislayer.h \
    dialog.h \
    osggistreeitem.h \
    filterstackdlg2.h

FORMS += mainwindow.ui \
         dialog.ui \
    FilterStackDlg2.ui
LIBS += -Lc:/msys/1.0/lib \
    -losg \
    -losgDB \
    -losgFX \
    -losgGA \
    -losgManipulator \
    -losgParticle \
    -losgSim \
    -losgTerrain \
    -losgText \
    -losgUtil \
    -losgViewer \
    -losgVolume \
    -losgWidget \
    -L"C:/msys/1.0/lib" \
    -lgdal \
    -L"../../lib" \
    -losgGISd \
    -L"./mini" \
    -lMini \
    -lgdal
INCLUDEPATH = c:/msys/1.0/include \
    "../../src/osgGIS" \
    "./mini" \
    "/usr/include"
