

INCLUDEPATH +=  $$PATH_TO_R2D \
                $$PATH_TO_R2D/UIWidgets \
                $$PATH_TO_R2D/Tools \
                $$PATH_TO_R2D/Events \
                $$PATH_TO_R2D/Events/UI \
                $$PATH_TO_R2D/GraphicElements \
                $$PATH_TO_R2D/ModelViewItems \
                $$PATH_TO_COMMON/Workflow \
                $$PATH_TO_COMMON/Workflow/WORKFLOW \
                $$PATH_TO_COMMON/Workflow/EXECUTION \
                $$PATH_TO_COMMON/RandomVariables \


SOURCES +=  $$PATH_TO_R2D/Tools/CSVReaderWriter.cpp \
            $$PATH_TO_R2D/Tools/XMLAdaptor.cpp \
            $$PATH_TO_R2D/Tools/AssetInputDelegate.cpp \
            $$PATH_TO_R2D/Tools/ComponentDatabase.cpp \
            $$PATH_TO_R2D/Tools/TablePrinter.cpp \
            $$PATH_TO_R2D/Events/UI/SiteConfig.cpp \
            $$PATH_TO_R2D/Events/UI/site.cpp \
            $$PATH_TO_R2D/Events/UI/SiteGrid.cpp \
            $$PATH_TO_R2D/Events/UI/GridDivision.cpp \
            $$PATH_TO_R2D/Events/UI/Location.cpp \
            $$PATH_TO_R2D/UIWidgets/PopUpWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/LoadResultsDialog.cpp \
            $$PATH_TO_R2D/GraphicElements/ConvexHull.cpp \
            $$PATH_TO_R2D/GraphicElements/PolygonBoundary.cpp \
            $$PATH_TO_R2D/GraphicElements/GridNode.cpp \
            $$PATH_TO_R2D/GraphicElements/NodeHandle.cpp \
            $$PATH_TO_R2D/GraphicElements/RectangleGrid.cpp \
            $$PATH_TO_R2D/UIWidgets/VisualizationWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/GasPipelineInputWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/ComponentInputWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/MapViewSubWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/ShakeMapWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/SimCenterMapGraphicsView.cpp \
            $$PATH_TO_R2D/UIWidgets/EmbeddedMapViewWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/GroundMotionStation.cpp \
            $$PATH_TO_R2D/UIWidgets/GroundMotionTimeHistory.cpp \
            $$PATH_TO_R2D/UIWidgets/RendererTableView.cpp \
            $$PATH_TO_R2D/UIWidgets/RendererModel.cpp \
            $$PATH_TO_R2D/UIWidgets/LayerManagerDialog.cpp \
            $$PATH_TO_R2D/UIWidgets/LayerManagerTableView.cpp \
            $$PATH_TO_R2D/UIWidgets/LayerManagerModel.cpp \
            $$PATH_TO_R2D/UIWidgets/LayerComboBoxItemDelegate.cpp \
            $$PATH_TO_R2D/UIWidgets/RendererComboBoxItemDelegate.cpp \
            $$PATH_TO_R2D/UIWidgets/ColorDialogDelegate.cpp \
            $$PATH_TO_R2D/ModelViewItems/TreeItem.cpp \
            $$PATH_TO_R2D/ModelViewItems/LayerTreeItem.cpp \
            $$PATH_TO_R2D/ModelViewItems/ListTreeModel.cpp \
            $$PATH_TO_R2D/ModelViewItems/LayerTreeModel.cpp \
            $$PATH_TO_R2D/ModelViewItems/LayerTreeView.cpp \
            $$PATH_TO_R2D/ModelViewItems/TreeViewStyle.cpp \
            $$PATH_TO_R2D/ModelViewItems/CustomListWidget.cpp \
            $$PATH_TO_R2D/ModelViewItems/CheckableTreeModel.cpp \
            $$PATH_TO_R2D/ModelViewItems/MutuallyExclusiveListWidget.cpp \
            $$PATH_TO_R2D/ModelViewItems/GISLegendView.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComponentSelection.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/CustomizedItemModel.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/Application.cpp \


HEADERS +=  $$PATH_TO_R2D/Tools/XMLAdaptor.h \
            $$PATH_TO_R2D/Tools/CSVReaderWriter.h \
            $$PATH_TO_R2D/Tools/AssetInputDelegate.h \
            $$PATH_TO_R2D/Tools/ComponentDatabase.h \
            $$PATH_TO_R2D/Tools/CustomListWidget.h \
            $$PATH_TO_R2D/Tools/TablePrinter.h \
            $$PATH_TO_R2D/Events/UI/SiteConfig.h \
            $$PATH_TO_R2D/Events/UI/site.h \
            $$PATH_TO_R2D/Events/UI/SiteGrid.h \
            $$PATH_TO_R2D/Events/UI/GridDivision.h \
            $$PATH_TO_R2D/Events/UI/Location.h \
            $$PATH_TO_R2D/UIWidgets/PopUpWidget.h \
            $$PATH_TO_R2D/UIWidgets/LoadResultsDialog.h \
            $$PATH_TO_R2D/GraphicElements/ConvexHull.h \
            $$PATH_TO_R2D/GraphicElements/PolygonBoundary.h \
            $$PATH_TO_R2D/GraphicElements/GridNode.h \
            $$PATH_TO_R2D/GraphicElements/NodeHandle.h \
            $$PATH_TO_R2D/GraphicElements/RectangleGrid.h \
            $$PATH_TO_R2D/UIWidgets/VisualizationWidget.h \
            $$PATH_TO_R2D/UIWidgets/GasPipelineInputWidget.h \
            $$PATH_TO_R2D/UIWidgets/ComponentInputWidget.h \
            $$PATH_TO_R2D/UIWidgets/MapViewSubWidget.h \
            $$PATH_TO_R2D/UIWidgets/ShakeMapWidget.h \
            $$PATH_TO_R2D/UIWidgets/SimCenterMapGraphicsView.h \
            $$PATH_TO_R2D/UIWidgets/EmbeddedMapViewWidget.h \
            $$PATH_TO_R2D/UIWidgets/GroundMotionStation.h \
            $$PATH_TO_R2D/UIWidgets/GroundMotionTimeHistory.h \
            $$PATH_TO_R2D/UIWidgets/RendererTableView.h \
            $$PATH_TO_R2D/UIWidgets/RendererModel.h \
            $$PATH_TO_R2D/UIWidgets/LayerComboBoxItemDelegate.h \
            $$PATH_TO_R2D/UIWidgets/RendererComboBoxItemDelegate.h \
            $$PATH_TO_R2D/UIWidgets/ColorDialogDelegate.h \
            $$PATH_TO_R2D/UIWidgets/LayerManagerDialog.h \
            $$PATH_TO_R2D/UIWidgets/LayerManagerTableView.h \
            $$PATH_TO_R2D/UIWidgets/LayerManagerModel.h \
            $$PATH_TO_R2D/ModelViewItems/TreeItem.h \
            $$PATH_TO_R2D/ModelViewItems/LayerTreeItem.h \
            $$PATH_TO_R2D/ModelViewItems/ListTreeModel.h \
            $$PATH_TO_R2D/ModelViewItems/LayerTreeModel.h \
            $$PATH_TO_R2D/ModelViewItems/LayerTreeView.h \
            $$PATH_TO_R2D/ModelViewItems/TreeViewStyle.h \
            $$PATH_TO_R2D/ModelViewItems/CustomListWidget.h \
            $$PATH_TO_R2D/ModelViewItems/CheckableTreeModel.h \
            $$PATH_TO_R2D/ModelViewItems/MutuallyExclusiveListWidget.h \
            $$PATH_TO_R2D/ModelViewItems/GISLegendView.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComponentSelection.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/CustomizedItemModel.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/Application.h \
