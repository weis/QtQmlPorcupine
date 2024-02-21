QT += quick multimedia

CONFIG += c++17 qmltypes

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QML_IMPORT_NAME = Porcupine.Components
QML_IMPORT_MAJOR_VERSION = 1
QML_IMPORT_MINOR_VERSION = 0
QML_IMPORT_PATH = $$OUT_PWD

macx: {
   QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
   QMAKE_INFO_PLIST = $$PWD/mac/osx/Info.plist
 }

INCLUDEPATH += Porcupine/Components $$PWD/src $$PWD/extern/porcupine/include
qml.path = $$PWD/ui/

SOURCES += \
        main.cpp \
        src/porcupine.cpp \
        src/qmlporcupine.cpp

RESOURCES += qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += $$PWD/porcupine/include

HEADERS += \
    src/porcupine.h \
    src/porcupine_fn.hpp \
    src/qmlporcupine.h

#############################################
# Library root locations
PV_LIBS_ROOT = $${PWD}/extern/lib

win32 {
    PV_LIBS_FILE = $${PV_LIBS_ROOT}/windows/amd64/libpv_porcupine.dll
}

macx: {
    # universal arm64 x86_64 architecture
    # run first build-mac-lib-universal.sh from $${PV_LIBS_ROOT}/mac
    PV_LIBS_FILE = $${PV_LIBS_ROOT}/mac/universal/libpv_porcupine.dylib
}

android: {
}

ios: {
}

unix:!macx:!ios: {
}


# Copy porcubine runtime libraries
win32 {
    CONFIG(debug, debug|release) {
        EXE_DIR = $${OUT_PWD}/debug
    } else {
        EXE_DIR = $${OUT_PWD}/release
    }

    defineTest(copyToDestdir) {
        files = $$1

        for(FILE, files) {
            DDIR = $$EXE_DIR

            # Replace slashes in paths with backslashes for Windows
            win32:FILE ~= s,/,\\,g
            win32:DDIR ~= s,/,\\,g

            QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
        }

        export(QMAKE_POST_LINK)
    }
    copyToDestdir($$PV_LIBS_FILE)
}

mac {
    LIBFILE_OUT = $${OUT_PWD}/QtPorcupine.app/Contents/MacOS/libpv_porcupine.dylib

    defineTest(copyToDestdir) {
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PV_LIBS_FILE) $$quote($$LIBFILE_OUT) $$escape_expand(\\n\\t)
        export(QMAKE_POST_LINK)
    }
    copyToDestdir()
}


DISTFILES += \
    data/picovoice/build-mac-lib-universal.sh \
    mac/osx/Info.plist \
