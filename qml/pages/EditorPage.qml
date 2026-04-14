import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Item {
    id: editor
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

    ColorDialog {
        id: backgroundColorDialog
        selectedColor: appController.project.backgroundColor
        onAccepted: {
            appController.project.backgroundMode = "color"
            appController.project.backgroundColor = selectedColor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Label { text: "编辑页"; font.pixelSize: 24; font.bold: true }
            Item { Layout.fillWidth: true }

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
                        model: ["#FFFFFF", "#F7F4EC", "#EFE4D2", "#E7E5E4", "#DCE5DC", "#DDE6F0", "#EADADA", "#2F3338"]
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
                    Button {
                        text: "自定义颜色"
                        onClicked: backgroundColorDialog.open()
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: appController.project.backgroundMode === "texture"
                    spacing: 6
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "纹理素材"; font.bold: true }
                        Item { Layout.fillWidth: true }
                        Button { text: "打开纹理文件夹"; onClicked: appController.openTextureDirectory() }
                        Button { text: "刷新纹理列表"; onClicked: appController.refreshTextures() }
                    }
                    Label { text: "目录：" + appController.textureDirectory; color: "#768294"; elide: Text.ElideMiddle; Layout.fillWidth: true }
                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 86
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
                            width: 112
                            height: 72
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
