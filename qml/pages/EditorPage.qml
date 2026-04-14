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

    Dialog {
        id: backgroundColorDialog
        parent: Overlay.overlay
        modal: true
        width: 420
        height: 320
        title: "自定义颜色"
        property color draftColor: appController.project.backgroundColor
        onOpened: {
            draftColor = appController.project.backgroundColor
            hueSlider.value = Math.max(0, draftColor.hsvHue)
            satSlider.value = Math.max(0, draftColor.hsvSaturation)
            valSlider.value = Math.max(0, draftColor.hsvValue)
            hexField.text = draftColor.toString()
        }
        standardButtons: Dialog.NoButton
        footer: RowLayout {
            spacing: 8
            Button { text: "取消"; onClicked: backgroundColorDialog.close() }
            Button {
                text: "确定"
                onClicked: {
                    appController.project.backgroundMode = "color"
                    appController.project.backgroundColor = backgroundColorDialog.draftColor
                    backgroundColorDialog.close()
                }
            }
        }
        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 44; radius: 8; color: backgroundColorDialog.draftColor; border.color: "#c7d0da" }
            Label { text: "色相" }
            Slider { id: hueSlider; Layout.fillWidth: true; from: 0; to: 1; onMoved: backgroundColorDialog.draftColor = Qt.hsva(value, satSlider.value, valSlider.value, 1.0) }
            Label { text: "饱和度" }
            Slider { id: satSlider; Layout.fillWidth: true; from: 0; to: 1; onMoved: backgroundColorDialog.draftColor = Qt.hsva(hueSlider.value, value, valSlider.value, 1.0) }
            Label { text: "明度" }
            Slider { id: valSlider; Layout.fillWidth: true; from: 0; to: 1; onMoved: backgroundColorDialog.draftColor = Qt.hsva(hueSlider.value, satSlider.value, value, 1.0) }
            RowLayout {
                Layout.fillWidth: true
                Label { text: "HEX" }
                TextField {
                    id: hexField
                    Layout.fillWidth: true
                    onEditingFinished: {
                        const c = Qt.color(text)
                        if (c.a > 0 || text.toLowerCase() === "#000000") {
                            backgroundColorDialog.draftColor = c
                        }
                        text = backgroundColorDialog.draftColor.toString()
                    }
                }
            }
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
                    RowLayout {
                        Layout.fillWidth: true
                        Button { text: "打开纹理文件夹"; onClicked: appController.openTextureDirectory() }
                        Button { text: "刷新纹理列表"; onClicked: appController.refreshTextures() }
                    }
                    ListView {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 70
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
