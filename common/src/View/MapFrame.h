/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_MapFrame
#define TrenchBroom_MapFrame

#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"
#include "View/Inspector.h"
#include "View/Selection.h"
#include "View/ViewTypes.h"

#include <QMainWindow>

#include <utility>

class QAction;
class QComboBox;
class QDropEvent;
class QMenuBar;
class QLabel;
class QSplitter;
class QTimer;

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
    }

    namespace View {
        class Action;
        class Autosaver;
        class Console;
        class InfoPanel;
        class FrameManager;
        class GLContextManager;
        class Inspector;
        class SwitchableMapViewContainer;
        class Tool;
        class ActionInfo;

        class MapFrame : public QMainWindow {
            Q_OBJECT
        private:
            FrameManager* m_frameManager;
            MapDocumentSPtr m_document;

            Autosaver* m_autosaver;
            QTimer* m_autosaveTimer;

            QSplitter* m_hSplitter;
            QSplitter* m_vSplitter;

            GLContextManager* m_contextManager;
            SwitchableMapViewContainer* m_mapView;
            InfoPanel* m_infoPanel;
            Console* m_console;
            Inspector* m_inspector;

            QComboBox* m_gridChoice;
            QLabel* m_statusBarLabel;
        private: // shortcuts
            using ActionMap = std::vector<std::pair<QAction*, const Action*>>;
            ActionMap m_actionMap;
        private: // special menu entries
            QMenu* m_recentDocumentsMenu;
            QAction* m_undoAction;
            QAction* m_redoAction;
            QAction* m_pasteAction;
            QAction* m_pasteAtOriginalPositionAction;
#if 0
            wxDialog* m_compilationDialog;
#endif
        public:
            MapFrame();
            MapFrame(FrameManager* frameManager, MapDocumentSPtr document);
            ~MapFrame() override;

            void positionOnScreen(QWidget* reference);
            MapDocumentSPtr document() const;
        public: // getters and such
            Logger& logger() const;
        public: // drop targets
            void setToolBoxDropTarget();
            void clearDropTarget();
        protected:
            void dragEnterEvent(QDragEnterEvent* event) override;
            void dropEvent(QDropEvent* event) override;
        private: // title bar contents
            void updateTitle();
        private: // menu bar
            class MenuBuilder;
            void createMenus();
            void updateShortcuts();
            void updateActionState();
            void updateUndoRedoActions();
            void updatePasteActions();

            void addRecentDocumentsMenu(QMenuBar* menuBar);
            void removeRecentDocumentsMenu(QMenuBar* menuBar);
            void updateRecentDocumentsMenu();
        private: // tool bar
            class ToolBarBuilder;
            void createToolBar();
        private: // status bar
            void createStatusBar();
            void updateStatusBar();
        private: // gui creation
            void createGui();
        private: // notification handlers
            void bindObservers();
            void unbindObservers();

            void documentWasCleared(View::MapDocument* document);
            void documentDidChange(View::MapDocument* document);
            void documentModificationStateDidChange();

            void transactionDone(const String&);
            void transactionUndone(const String&);

            void preferenceDidChange(const IO::Path& path);
            void gridDidChange();
            void toolActivated(Tool* tool);
            void toolDeactivated(Tool* tool);
            void selectionDidChange(const Selection& selection);
            void currentLayerDidChange(const TrenchBroom::Model::Layer* layer);
            void groupWasOpened(Model::Group* group);
            void groupWasClosed(Model::Group* group);
        private: // menu event handlers
            void bindEvents();
        private slots:
            void triggerAction(const Action& action);
        public:
            bool newDocument(Model::GameSPtr game, Model::MapFormat mapFormat);
            bool openDocument(Model::GameSPtr game, Model::MapFormat mapFormat, const IO::Path& path);
            bool saveDocument();
            bool saveDocumentAs();
            bool exportDocumentAsObj();
            bool exportDocument(Model::ExportFormat format, const IO::Path& path);
        private:
            bool confirmOrDiscardChanges();
        public:
            void loadPointFile();
            void reloadPointFile();
            void unloadPointFile();
            bool canReloadPointFile() const;
            bool canUnloadPortalFile() const;

            void loadPortalFile();
            void reloadPortalFile();
            void unloadPortalFile();
            bool canReloadPortalFile() const;
            bool canUnloadPointFile() const;

            void reloadTextureCollections();
            void reloadEntityDefinitions();
            void closeDocument();

            void undo();
            void redo();
            bool canUndo() const;
            bool canRedo() const;

            void repeatLastCommands();
            void clearRepeatableCommands();
            bool hasRepeatableCommands() const;

            void cutSelection();
            void copySelection();
            void copyToClipboard();
            bool canCutSelection() const;
            bool canCopySelection() const;

            void pasteAtCursorPosition();
            void pasteAtOriginalPosition();
            PasteType paste();
            bool canPaste() const;

            void duplicateSelection();
            bool canDuplicateSelectino() const;

            void deleteSelection();
            bool canDeleteSelection() const;

            void selectAll();
            void selectSiblings();
            void selectTouching();
            void selectInside();
            void selectTall();
            void selectByLineNumber();
            void selectNone();

            bool canSelect() const;
            bool canSelectSiblings() const;
            bool canSelectByBrush() const;
            bool canSelectTall() const;
            bool canDeselect() const;
            bool canChangeSelection() const;

            void groupSelectedObjects();
            bool canGroup() const;

            void ungroupSelectedObjects();
            bool canUngroup() const;

            void OnEditDeactivateTool();
            void toggleCreateComplexBrushTool();
            bool canToggleCreateComplexBrushTool() const;
            bool createComplexBrushToolActive() const;

            void toggleClipTool();
            bool canToggleClipTool() const;
            bool clipToolActive() const;

            void toggleRotateObjectsTool();
            bool canToggleRotateObjectsTool() const;
            bool rotateObjectsToolActive() const;

            void toggleScaleObjectsTool();
            bool canToggleScaleObjectsTool() const;
            bool scaleObjectsToolActive() const;

            void toggleShearObjectsTool();
            bool canToggleShearObjectsTool() const;
            bool shearObjectsToolActive() const;

            void toggleVertexTool();
            bool canToggleVertexTool() const;
            bool vertexToolActive() const;

            void toggleEdgeTool();
            bool canToggleEdgeTool() const;
            bool edgeToolActive() const;

            void toggleFaceTool();
            bool canToggleFaceTool() const;
            bool faceToolActive() const;

            void csgConvexMerge();
            bool canDoCsgConvexMerge() const;

            void csgSubtract();
            bool canDoCsgSubtract() const;

            void csgHollow();
            bool canDoCsgHollow() const;

            void csgIntersect();
            bool canDoCsgIntersect() const;

            void snapVerticesToInteger();
            void snapVerticesToGrid();
            bool canSnapVertices() const;

            void replaceTexture();

            void toggleTextureLock();
            void toggleUVLock();

            void toggleShowGrid();
            void toggleSnapToGrid();

            void incGridSize();
            bool canIncGridSize() const;

            void decGridSize();
            bool canDecGridSize() const;

            void setGridSize(int size);

            void moveCameraToNextPoint();
            bool canMoveCameraToNextPoint() const;

            void moveCameraToPreviousPoint();
            bool canMoveCameraToPreviousPoint() const;

            void focusCameraOnSelection();
            bool canFocusCamera() const;

            void moveCameraToPosition();

            void isolateSelection();
            bool canIsolateSelection() const;

            void hideSelection();
            bool canHideSelection() const;

            void showAll();

            void switchToInspectorPage(Inspector::InspectorPage page);

            void toggleInfoPanel();
            bool infoPanelVisible() const;

            void toggleInspector();
            bool inspectorVisible() const;

            void toggleMaximizeCurrentView();
            bool currentViewMaximized();

            void showCompileDialog();
            void compilationDialogWillClose();

            void showLaunchEngineDialog();

            void debugPrintVertices();
            void debugCreateBrush();
            void debugCreateCube();
            void debugClipBrush();
            void debugCopyJSShortcutMap();
            void debugCrash();
            void debugThrowExceptionDuringCommand();
            void debugSetWindowSize();

            void OnFlipObjectsHorizontally();
            void OnFlipObjectsVertically();

            void OnToolBarSetGridSize(int index);
            void onFocusChange(QWidget* old, QWidget* now);
        private:

            //wxTextCtrl* findFocusedTextCtrl() const;

            bool canCompile() const;
            bool canLaunch() const;
        private: // other event handlers
  //          void OnClose(wxCloseEvent& event);
            void OnAutosaveTimer();
        private: // grid helpers
            static int indexForGridSize(const int gridSize);
            static int gridSizeForIndex(const int index);
        };
    }
}

#endif /* defined(TrenchBroom_MapFrame) */
