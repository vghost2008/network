######################################################################
# Automatically generated by qmake (3.0) ?? 6? 16 16:06:36 2016
######################################################################

QT += network widgets
OBJECTS_DIR = temp
MOC_DIR = temp
INCLUDEPATH += .
DEFINES -= QT_NO_DEBUG_OUTPUT
unix {
	LIBS += -L${PROJECT_DIR}/release -lwlib \
	-Wl,-rpath=/usr/local/lib \
	-Wl,-rpath=/usr/lib
	CONFIG += debug
	INCLUDEPATH += ${PROJECT_DIR}/swlib/include
	DEFINES += POSIX_API
}
win32 {
	LIBS += -LE:\program\wlibV1\release -lwlib  \
	CONFIG += release console
	INCLUDEPATH += E:\program\wlibV1\include
}
QMAKE_CXXFLAGS += -std=c++11 -Wno-sign-compare -Wno-reorder

TEMPLATE = app
TARGET = simradar
# Input
HEADERS += maindlg.h trace_data.h
SOURCES += main.cpp maindlg.cpp trace_data.cpp
