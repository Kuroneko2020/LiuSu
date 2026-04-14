import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Item {
    id: editor
    property int currentPage: 1
    signal navigateRequested(int pageIndex)
    property color panelBg: "#ffffff"
    property color workbenchBg: "#e9edf2"
    property color accentColor: "#3b82f6"

    FileDialog {
        id: singleImportDialog
        title: "选择一张图片"
        fileMode: FileDialog.OpenFile
        nameFilters: ["图片文件 (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.heic *.heif)"]
        property int targetSlot: -1
        onAccepted: appController.importToSlotFromFile(targetSlot, selectedFile)
    }

    FileDialog {
        id: batchImportDialog
        title: "批量导入图片"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["图片文件 (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.heic *.heif)"]
        onAccepted: appController.batchImportFromFiles(selectedFiles)
    }

    ColorPickerDialog {
        id: backgroundColorDialog
        onConfirmed: (colorValue) => {
            appController.project.backgroundMode = "color"
            appController.project.backgroundColor = colorValue
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        PageTopBar {
            Layout.fillWidth: true
            currentPage: editor.currentPage
            onNavigate: (pageIndex) => editor.navigateRequested(pageIndex)
            IconToolButton {
                icon.source: "qrc:/qt/qml/PhotoTemplateEditor/res/icons/import.svg"
                tooltipText: "批量导入"
                onClicked: batchImportDialog.open()
            }
            IconToolButton {
                icon.source: "qrc:/qt/qml/PhotoTemplateEditor/res/icons/page-add.svg"
                tooltipText: "新建空白页"
                onClicked: appController.createBlankPageForCurrentTemplate()
            }
            IconToolButton {
                icon.source: "qrc:/qt/qml/PhotoTemplateEditor/res/icons/export-one.svg"
                tooltipText: "导出当前页"
                onClicked: appController.exportCurrentPage()
            }
            IconToolButton {
                icon.source: "qrc:/qt/qml/PhotoTemplateEditor/res/icons/export-all.svg"
                tooltipText: "导出全队列"
                onClicked: appController.exportQueue()
            }
            IconToolButton {
                icon.source: "qrc:/qt/qml/PhotoTemplateEditor/res/icons/page-delete.svg"
                tooltipText: "删除当前页"
                onClicked: appController.project.deleteCurrentPage()
            }
        }

        Frame {
            Layout.fillWidth: true
            background: Rectangle {
                radius: 10
                color: editor.panelBg
                border.color: "#d8e0ea"
            }
            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "背景"; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Button {
                        visible: appController.project.backgroundMode === "texture"
                        text: "打开纹理文件夹"
                        flat: true
                        implicitHeight: 34
                        background: Rectangle {
                            radius: 8
                            color: parent.down ? "#dbe6f5" : (parent.hovered ? "#ebf1fa" : "#f4f7fb")
                            border.color: "#d0daea"
                        }
                        onClicked: appController.openTextureDirectory()
                    }
                    Button {
                        visible: appController.project.backgroundMode === "texture"
                        text: "刷新纹理列表"
                        flat: true
                        implicitHeight: 34
                        background: Rectangle {
                            radius: 8
                            color: parent.down ? "#dbe6f5" : (parent.hovered ? "#ebf1fa" : "#f4f7fb")
                            border.color: "#d0daea"
                        }
                        onClicked: appController.refreshTextures()
                    }
                    Rectangle {
                        Layout.preferredWidth: 180
                        Layout.preferredHeight: 34
                        radius: 8
                        color: "#f0f4f8"
                        border.color: "#d7e0ea"
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 2
                            spacing: 2
                            Button {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: "纯色"
                                flat: true
                                background: Rectangle {
                                    radius: 6
                                    color: appController.project.backgroundMode === "color" ? editor.accentColor : "transparent"
                                }
                                contentItem: Text {
                                    text: parent.text
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    color: appController.project.backgroundMode === "color" ? "#ffffff" : "#4a5663"
                                }
                                onClicked: appController.project.backgroundMode = "color"
                            }
                            Button {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                text: "纹理"
                                flat: true
                                background: Rectangle {
                                    radius: 6
                                    color: appController.project.backgroundMode === "texture" ? editor.accentColor : "transparent"
                                }
                                contentItem: Text {
                                    text: parent.text
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    color: appController.project.backgroundMode === "texture" ? "#ffffff" : "#4a5663"
                                }
                                onClicked: appController.project.backgroundMode = "texture"
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: appController.project.backgroundMode === "color"
                    Repeater {
                        model: ["#FFFFFF", "#F7F4EC", "#EFE4D2", "#ECE7E1", "#E7E5E4", "#DCE5DC", "#DDE6F0", "#D8E3EA", "#DFE9E4", "#EADADA", "#D9D2C7", "#C9D0D8", "#8A929D", "#2F3338"]
                        delegate: Rectangle {
                            required property string modelData
                            width: 32
                            height: 32
                            radius: 8
                            color: modelData
                            border.width: appController.project.backgroundColor.toString().toLowerCase() === modelData.toLowerCase() ? 3 : 1
                            border.color: appController.project.backgroundColor.toString().toLowerCase() === modelData.toLowerCase() ? editor.accentColor : "#9aa6b3"
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    appController.project.backgroundMode = "color"
                                    appController.project.backgroundColor = modelData
                                }
                            }
                        }
                    }
                    Rectangle {
                        width: 32
                        height: 32
                        radius: 8
                        border.width: 1
                        border.color: "#8aa0bb"
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#ff5b8a" }
                            GradientStop { position: 0.25; color: "#ffb347" }
                            GradientStop { position: 0.5; color: "#6cd98b" }
                            GradientStop { position: 0.75; color: "#55a8ff" }
                            GradientStop { position: 1.0; color: "#b07bff" }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                backgroundColorDialog.selectedColor = appController.project.backgroundColor
                                backgroundColorDialog.open()
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: appController.project.backgroundMode === "texture"
                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 64
                        orientation: ListView.Horizontal
                        spacing: 8
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        model: {
                            appController.textureListRevision
                            return appController.availableTextures()
                        }
                        delegate: Rectangle {
                            required property string modelData
                            width: 90
                            height: 58
                            radius: 8
                            color: "#f8fafc"
                            border.width: appController.project.backgroundTexturePath === modelData ? 3 : 1
                            border.color: appController.project.backgroundTexturePath === modelData ? editor.accentColor : "#c7d0da"
                            Image {
                                anchors.fill: parent
                                anchors.margins: 2
                                source: modelData
                                fillMode: Image.PreserveAspectCrop
                                smooth: true
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    appController.project.backgroundMode = "texture"
                                    appController.project.backgroundTexturePath = modelData
                                }
                            }
                        }
                        Label {
                            anchors.centerIn: parent
                            visible: parent.count === 0
                            text: "纹理目录暂无可用图片（png/jpg/jpeg/webp/bmp）"
                            color: "#6f7b88"
                        }
                        WheelHandler {
                            onWheel: (event) => {
                                parent.contentX = Math.max(0, Math.min(parent.contentWidth - parent.width, parent.contentX - event.angleDelta.y))
                                event.accepted = true
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: previewContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: editor.workbenchBg
            border.color: "#d0d8e2"

            TemplateCanvas {
                id: templateCanvas
                anchors.centerIn: parent
                width: Math.min(parent.width * 0.88, 780)
                height: width / appController.pageAspectRatio
                onImportSlotRequested: (slotIndex) => {
                    singleImportDialog.targetSlot = slotIndex
                    singleImportDialog.open()
                }
            }
        }

        FilmStrip {
            Layout.fillWidth: true
            Layout.preferredHeight: 76
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        propagateComposedEvents: true
        onPressed: (mouse) => {
            const scenePos = editor.mapToItem(null, mouse.x, mouse.y)
            const inPreview = mouse.x >= previewContainer.x
                              && mouse.x <= previewContainer.x + previewContainer.width
                              && mouse.y >= previewContainer.y
                              && mouse.y <= previewContainer.y + previewContainer.height
            const clickedSlot = inPreview ? templateCanvas.slotIndexAt(scenePos) : -1
            if (!inPreview || clickedSlot < 0) {
                appController.project.clearSelection()
            }
            mouse.accepted = false
        }
    }
}
