import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Item {
    id: editor

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
        spacing: 12

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
            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Label { text: "背景"; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Button {
                        text: "纯色"
                        checkable: true
                        checked: appController.project.backgroundMode === "color"
                        onClicked: appController.project.backgroundMode = "color"
                    }
                    Button {
                        text: "纹理"
                        checkable: true
                        checked: appController.project.backgroundMode === "texture"
                        onClicked: appController.project.backgroundMode = "texture"
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    visible: appController.project.backgroundMode === "color"
                    Repeater {
                        model: ["#FFFFFF", "#F7F4EC", "#EFE4D2", "#E7E5E4", "#DCE5DC", "#DDE6F0", "#EADADA", "#2F3338"]
                        delegate: Rectangle {
                            required property string modelData
                            width: 20
                            height: 20
                            radius: 4
                            color: modelData
                            border.color: "#9aa6b3"
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
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "纹理目录：" + appController.textureDirectory; elide: Text.ElideMiddle; Layout.fillWidth: true }
                        Button { text: "打开纹理文件夹"; onClicked: appController.openTextureDirectory() }
                        Button { text: "刷新纹理列表"; onClicked: appController.refreshTextures() }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Repeater {
                            model: {
                                appController.textureListRevision
                                return appController.availableTextures()
                            }
                            delegate: Rectangle {
                                required property string modelData
                                width: 54
                                height: 36
                                color: "#f0f2f4"
                                border.color: "#c7d0da"
                                Image {
                                    anchors.fill: parent
                                    anchors.margins: 1
                                    source: modelData
                                    fillMode: Image.Stretch
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        appController.project.backgroundMode = "texture"
                                        appController.project.backgroundTexturePath = modelData
                                    }
                                }
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
            radius: 10
            color: "#f5f7fa"
            border.color: "#d4dbe3"

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
