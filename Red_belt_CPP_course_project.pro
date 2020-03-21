TEMPLATE = app
CONFIG += console c++1z
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        parse.cpp \
        search_server.cpp

HEADERS += \
    iterator_range.h \
    parse.h \
    profile.h \
    search_server.h \
    test_runner.h

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread
