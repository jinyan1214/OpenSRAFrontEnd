#ifndef UserInputCPTWidget_H
#define UserInputCPTWidget_H
/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Stevan Gavrilovic

#include "GISSelectable.h"

#include <qgsfeature.h>

#include "SimCenterAppWidget.h"

#include <memory>

#include <QMap>

class QGISVisualizationWidget;
class VisualizationWidget;
class SimCenterUnitsWidget;
class ComponentTableView;
class AssetInputDelegate;
class ComponentDatabase;

class QStackedWidget;
class QLineEdit;
class QProgressBar;
class QLabel;

class UserInputCPTWidget : public SimCenterAppWidget, public GISSelectable
{
    Q_OBJECT

public:
    UserInputCPTWidget(VisualizationWidget* visWidget, QWidget *parent = nullptr);
    ~UserInputCPTWidget();

    void insertSelectedAssets(QgsFeatureIds& featureIds);
    void clearSelectedAssets(void);

    QStackedWidget* getUserInputCPTWidget(void);

    bool inputFromJSON(QJsonObject &jsonObject);
    bool outputToJSON(QJsonObject &jsonObj);
    bool inputAppDataFromJSON(QJsonObject &jsonObj);
    bool outputAppDataToJSON(QJsonObject &jsonObj);
    bool copyFiles(QString &destDir);
    void clear(void);

public slots:
    void handleComponentSelection(void);
    void showUserGMSelectDialog(void);

private slots:

    void loadUserCPTData(void);
    void chooseEventFileDialog(void);
    void chooseMotionDirDialog(void);
    void handleRowSelect(const QModelIndex &index);

signals:
    void outputDirectoryPathChanged(QString motionDir, QString eventFile);
    void eventTypeChangedSignal(QString eventType);
    void loadingComplete(const bool value);

private slots:
    void selectComponents(void);
    void clearComponentSelection(void);

private:
    void showProgressBar(void);
    void hideProgressBar(void);

    QStackedWidget* theStackedWidget = nullptr;
    ComponentDatabase*  theComponentDb = nullptr;

    ComponentTableView* siteListTableWidget = nullptr;
    ComponentTableView* siteDataTableWidget = nullptr;

    QGISVisualizationWidget* theVisualizationWidget = nullptr;

    QgsVectorLayer* mainLayer = nullptr;
    QgsVectorLayer* selectedFeaturesLayer = nullptr;

    QString eventFile;
    QString cptDataDir;

    AssetInputDelegate* selectComponentsLineEdit = nullptr;

    QLineEdit *CPTSitesFileLineEdit = nullptr;
    QLineEdit *CPTDirLineEdit = nullptr;

    QLabel* progressLabel = nullptr;
    QWidget* progressBarWidget = nullptr;
    QWidget* fileInputWidget = nullptr;
    QProgressBar* progressBar = nullptr;

    QMap<QString,QVector<QStringList>> stationMap;

};

#endif // UserInputCPTWidget_H
