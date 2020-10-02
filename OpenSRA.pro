

QT += core gui charts concurrent network sql webenginewidgets webengine webchannel 3dcore 3drender 3dextras charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

TARGET = OpenSRA
TEMPLATE = app

VERSION=1.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

equals(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 15) {
        error("$$TARGET requires Qt 5.15.0")
    }
        equals(QT_MINOR_VERSION, 15) : lessThan(QT_PATCH_VERSION, 0) {
                error("$$TARGET requires Qt 5.15.0")
        }
}

ARCGIS_RUNTIME_VERSION = 100.9
include($$PWD/arcgisruntime.pri)

win32 {
    RC_ICONS = icons/NHERI-EEUQ-Icon.ico
} else {
    mac {
    ICON = icons/NHERI-EEUQ-Icon.icns
    }
}


SOURCES += main.cpp \
    WorkflowAppOpenSRA.cpp \
    SimCenterWidget.cpp \
    SimCenterPreferences.cpp \
    SimCenterComponentSelection.cpp \
    HeaderWidget.cpp \
    SimCenterAppWidget.cpp \
    sectiontitle.cpp \
    CustomizedItemModel.cpp \
    WorkflowAppWidget.cpp \
    MainWindowWorkflowApp.cpp \
    UIWidgets/DecisionVariableWidget.cpp \
    UIWidgets/SourceCharacterizationWidget.cpp \
    UIWidgets/ResultsWidget.cpp \
    UIWidgets/PipelineNetworkWidget.cpp \
    UIWidgets/EngDemandParamWidget.cpp \
    UIWidgets/GeneralInformationWidget.cpp \
    UIWidgets/IntensityMeasureWidget.cpp \
    UIWidgets/DamageMeasureWidget.cpp \
    Utils/dialogabout.cpp \
    Utils/RelativePathResolver.cpp \
    RunWidget.cpp

HEADERS  += \
    WorkflowAppOpenSRA.h \
    SimCenterWidget.h \
    SimCenterPreferences.h \
    SimCenterComponentSelection.h \
    SimCenterAppWidget.h \
    HeaderWidget.h \
    DecisionVariableWidget.h \
    sectiontitle.h \
    CustomizedItemModel.h \
    WorkflowAppWidget.h \
    MainWindowWorkflowApp.h \
    RunWidget.h \
    Utils/dialogabout.h \
    Utils/RelativePathResolver.h \
    UIWidgets/DecisionVariableWidget.h \
    UIWidgets/SourceCharacterizationWidget.h \
    UIWidgets/ResultsWidget.h \
    UIWidgets/PipelineNetworkWidget.h \
    UIWidgets/EngDemandParamWidget.h \
    UIWidgets/GeneralInformationWidget.h \
    UIWidgets/IntensityMeasureWidget.h \
    UIWidgets/DamageMeasureWidget.h \
    Utils/dialogabout.h \
    Utils/RelativePathResolver.h \

RESOURCES += \
    images.qrc \
    styles.qrc

INCLUDEPATH += $$PWD/Utils \
               $$PWD/styles \
               $$PWD/UIWidgets \

