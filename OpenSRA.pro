

QT += core gui charts concurrent network sql webenginewidgets webengine webchannel 3dcore 3drender 3dextras charts xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

TARGET = OpenSRA
TEMPLATE = app
VERSION=0.0.2
DEFINES += APP_VERSION=\\\"$$VERSION\\\" OpenSRA

# C++17 support
CONFIG += c++14

# Full optimization on release
QMAKE_CXXFLAGS_RELEASE += -O3


# Check for the required Qt version
equals(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 15) {
        error("$$TARGET requires Qt 5.15.0")
    }
        equals(QT_MINOR_VERSION, 15) : lessThan(QT_PATCH_VERSION, 0) {
                error("$$TARGET requires Qt 5.15.0")
        }
}

mac {
#Deployment target should be lower than SDK to make plugins useable in an older OS
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

}

#Application Icons
win32 {
    RC_ICONS = icons/openSRA-icon.ico
} else {
    mac {
    ICON = icons/openSRA-icon.icns
    }
}


ARCGIS_RUNTIME_VERSION = 100.9
include($$PWD/arcgisruntime.pri)


# Specify the path to R2D and common
PATH_TO_R2D=../../R2DTool/R2DTool
PATH_TO_COMMON=../../SimCenterCommon

# To avoid code copying, include the common SimCenter code
include(OpenSRACommon.pri)
include($$PATH_TO_COMMON/Common/Common.pri)

SOURCES += main.cpp \
    UIWidgets/DMPipeStrainWidget.cpp \
    UIWidgets/DVNumRepairsWidget.cpp \
    UIWidgets/EDPGroundSettlementWidget.cpp \
    UIWidgets/EDPGroundStrainWidget.cpp \
    UIWidgets/EDPLandslideWidget.cpp \
    UIWidgets/EDPLiquefactionWidget.cpp \
    UIWidgets/EDPLatSpreadWidget.cpp \
    UIWidgets/EDPSubSurfFaultRupWidget.cpp \
    UIWidgets/EDPSurfFaultRupWidget.cpp \
    UIWidgets/MonteCarloSamplingWidget.cpp \
    UIWidgets/FixedResidualsSamplingWidget.cpp \
    UIWidgets/OpenSHAWidget.cpp \
    UIWidgets/OpenSRAPostProcessor.cpp \
    UIWidgets/ResultsWidget.cpp \
    UIWidgets/UncertaintyQuantificationWidget.cpp \
    WorkflowAppOpenSRA.cpp \
    WorkflowAppWidget.cpp \
    MainWindowWorkflowApp.cpp \
    LocalApplication.cpp \
    OpenSRAPreferences.cpp \
    UIWidgets/DecisionVariableWidget.cpp \
    UIWidgets/SourceCharacterizationWidget.cpp \
    UIWidgets/CustomVisualizationWidget.cpp \
    UIWidgets/PipelineNetworkWidget.cpp \
    UIWidgets/EngDemandParamWidget.cpp \
    UIWidgets/GeneralInformationWidget.cpp \
    UIWidgets/IntensityMeasureWidget.cpp \
    UIWidgets/DamageMeasureWidget.cpp \
    RunWidget.cpp \

HEADERS  += \
    OpenSRAUserPass.h \
    UIWidgets/DMPipeStrainWidget.h \
    UIWidgets/DVNumRepairsWidget.h \
    UIWidgets/EDPGroundSettlementWidget.h \
    UIWidgets/EDPGroundStrainWidget.h \
    UIWidgets/EDPLandslideWidget.h \
    UIWidgets/EDPLiquefactionWidget.h \
    UIWidgets/EDPLatSpreadWidget.h \
    UIWidgets/EDPSubSurfFaultRupWidget.h \
    UIWidgets/EDPSurfFaultRupWidget.h \
    UIWidgets/MonteCarloSamplingWidget.h \
    UIWidgets/FixedResidualsSamplingWidget.h \
    UIWidgets/OpenSHAWidget.h \
    UIWidgets/OpenSRAPostProcessor.h \
    UIWidgets/ResultsWidget.h \
    UIWidgets/UncertaintyQuantificationWidget.h \
    WorkflowAppOpenSRA.h \
    WorkflowAppWidget.h \
    MainWindowWorkflowApp.h \
    LocalApplication.h \
    OpenSRAPreferences.h \
    RunWidget.h \
    UIWidgets/DecisionVariableWidget.h \
    UIWidgets/SourceCharacterizationWidget.h \
    UIWidgets/CustomVisualizationWidget.h \
    UIWidgets/PipelineNetworkWidget.h \
    UIWidgets/EngDemandParamWidget.h \
    UIWidgets/GeneralInformationWidget.h \
    UIWidgets/IntensityMeasureWidget.h \
    UIWidgets/DamageMeasureWidget.h \

RESOURCES += \
    images.qrc \
    styles.qrc

INCLUDEPATH += $$PWD/Utils \
               $$PWD/styles \
               $$PWD/UIWidgets \


# Copies over the examples folder into the build directory
win32 {
DESTDIR = $$shell_path($$OUT_PWD)
Release:DESTDIR = $$DESTDIR/release
Debug:DESTDIR = $$DESTDIR/debug

message($$DESTDIR)

PATH_TO_BINARY=$$DESTDIR/Examples
} else {
    mac {
    PATH_TO_BINARY=$$OUT_PWD/OpenSRA.app/Contents/MacOS
    }
}

copydata.commands = $(COPY_DIR) \"$$shell_path($$PWD/Examples)\" \"$$shell_path($$PATH_TO_BINARY)\"
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

QMAKE_CXXFLAGS += -mmacosx-version-min=10.14

