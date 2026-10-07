import QtQuick
import LiuSu
import QtQuick.Controls
import QtQuick.Dialogs

// 编辑页（J 稿）：顶栏工具 + 背景行 + 坐标纸工作台 + 胶片栏（页队列）。
// UI 只做展示与交互；所有几何与像素真相在 domain / render 层。
Item {
    id: editor

    signal requestHome()

    // ---- 文件与颜色对话框 ----
    FileDialog {
        id: batchImportDialog
        title: qsTr("批量导入照片到空槽位")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("图片文件 (*.jpg *.jpeg *.png *.webp *.bmp)")]
        onAccepted: app.assignFilesToEmptySlots(selectedFiles)
    }
    FileDialog {
        id: replaceDialog
        title: qsTr("选择照片")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("图片文件 (*.jpg *.jpeg *.png *.webp *.bmp)")]
        property int slotIndex: -1
        onAccepted: app.assignFileToSlot(slotIndex, selectedFile)
    }
    ColorDialog {
        id: backgroundDialog
        title: qsTr("自定义背景颜色")
        onAccepted: app.setBackground(color.toString().toLowerCase())
    }
    FileDialog {
        id: openProjectDialog
        title: qsTr("打开项目")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("留素项目 (*.liusu)"), qsTr("所有文件 (*)")]
        onAccepted: app.openProject(selectedFile)
    }
    FileDialog {
        id: saveProjectDialog
        title: qsTr("保存项目")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "liusu"
        nameFilters: [qsTr("留素项目 (*.liusu)")]
        onAccepted: app.saveProject(selectedFile)
    }
    FileDialog {
        id: exportPageDialog
        title: qsTr("导出当前页")
        fileMode: FileDialog.SaveFile
        defaultSuffix: exportSettingsDialog.jpeg ? "jpg" : "png"
        nameFilters: exportSettingsDialog.jpeg ? [qsTr("JPEG 图片 (*.jpg)")] : [qsTr("PNG 图片 (*.png)")]
        onAccepted: app.exportCurrentPage(selectedFile, exportSettingsDialog.ppi,
                                          exportSettingsDialog.jpeg, exportSettingsDialog.quality)
    }
    FolderDialog {
        id: exportAllDialog
        title: qsTr("选择导出目录（导出全部页）")
        onAccepted: app.exportAllPages(selectedFolder, 300, true, 95)
    }

    // ---- 导出设置弹窗（亚克力板 + 背后磨砂）----
    Dialog {
        id: exportSettingsDialog
        anchors.centerIn: Overlay.overlay
        width: 440
        padding: 0
        modal: true
        closePolicy: Popup.CloseOnEscape

        property int ppi: 300
        property bool jpeg: true
        property int quality: 95
        property bool customPpi: false

        Overlay.modal: Rectangle { color: Qt.rgba(237 / 255, 234 / 255, 228 / 255, 0.45) }

        background: AcrylicPanel {
            cornerRadius: 2
            showScrews: true
        }

        contentItem: Column {
            spacing: 0
            padding: 22

            Row {
                spacing: 10
                Text {
                    text: qsTr("导出当前页")
                    color: AppTheme.ink
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 16
                    font.bold: true
                    font.letterSpacing: 2
                }
                Text {
                    text: "EXPORT · " + app.pageLabel
                    color: AppTheme.ink3
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 10
                    font.bold: true
                    font.letterSpacing: 2.4
                    anchors.baseline: parent.children[0].baseline
                }
            }
            Text {
                topPadding: 10
                text: qsTr("画布 148 × 100 MM · %1 PPI · 约 %2 × %3 PX")
                        .arg(exportSettingsDialog.ppi)
                        .arg(Math.round(148 / 25.4 * exportSettingsDialog.ppi))
                        .arg(Math.round(100 / 25.4 * exportSettingsDialog.ppi))
                color: AppTheme.ink2
                font.family: AppTheme.fontFamily
                font.pixelSize: 12
                font.letterSpacing: 0.8
            }

            // 格式
            Item {
                width: parent.width - 44
                height: 44
                Text {
                    text: qsTr("FORMAT 格式")
                    color: AppTheme.ink2
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 11
                    font.bold: true
                    font.letterSpacing: 1.5
                    anchors.verticalCenter: parent.verticalCenter
                }
                Row {
                    anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                    spacing: 0
                    Rectangle {
                        width: 64; height: 30
                        color: exportSettingsDialog.jpeg ? AppTheme.ink : "transparent"
                        border.width: exportSettingsDialog.jpeg ? 0 : 1
                        border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.2)
                        Text { anchors.centerIn: parent; text: "JPEG"; color: exportSettingsDialog.jpeg ? "#f4f1ea" : AppTheme.ink2; font.pixelSize: 12; font.bold: true; font.family: AppTheme.fontFamily }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: exportSettingsDialog.jpeg = true }
                    }
                    Rectangle {
                        width: 64; height: 30
                        color: !exportSettingsDialog.jpeg ? AppTheme.ink : "transparent"
                        border.width: !exportSettingsDialog.jpeg ? 0 : 1
                        border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.2)
                        Text { anchors.centerIn: parent; text: "PNG"; color: !exportSettingsDialog.jpeg ? "#f4f1ea" : AppTheme.ink2; font.pixelSize: 12; font.bold: true; font.family: AppTheme.fontFamily }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: exportSettingsDialog.jpeg = false }
                    }
                }
            }
            Rectangle { width: parent.width - 44; height: 1; color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.07) }

            // 分辨率
            Item {
                width: parent.width - 44
                height: 44
                Text {
                    text: qsTr("RESOLUTION 分辨率")
                    color: AppTheme.ink2
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 11
                    font.bold: true
                    font.letterSpacing: 1.5
                    anchors.verticalCenter: parent.verticalCenter
                }
                Row {
                    anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                    spacing: 0
                    Repeater {
                        model: [{ label: "300", value: 300 }, { label: "600", value: 600 }]
                        delegate: Rectangle {
                            required property var modelData
                            width: 64; height: 30
                            color: (!exportSettingsDialog.customPpi && exportSettingsDialog.ppi === modelData.value) ? AppTheme.ink : "transparent"
                            border.width: (!exportSettingsDialog.customPpi && exportSettingsDialog.ppi === modelData.value) ? 0 : 1
                            border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.2)
                            Text { anchors.centerIn: parent; text: modelData.label; color: (!exportSettingsDialog.customPpi && exportSettingsDialog.ppi === modelData.value) ? "#f4f1ea" : AppTheme.ink2; font.pixelSize: 12; font.bold: true; font.family: AppTheme.fontFamily }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { exportSettingsDialog.customPpi = false; exportSettingsDialog.ppi = modelData.value } }
                        }
                    }
                    Rectangle {
                        width: 70; height: 30
                        color: exportSettingsDialog.customPpi ? AppTheme.ink : "transparent"
                        border.width: exportSettingsDialog.customPpi ? 0 : 1
                        border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.2)
                        Text { anchors.centerIn: parent; text: qsTr("自定义"); color: exportSettingsDialog.customPpi ? "#f4f1ea" : AppTheme.ink2; font.pixelSize: 12; font.bold: true; font.family: AppTheme.fontFamily }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: exportSettingsDialog.customPpi = true }
                    }
                }
            }
            // 自定义 PPI 输入（合法范围 72–1200，ADR-0004）
            TextField {
                width: 120
                height: 32
                visible: exportSettingsDialog.customPpi
                text: exportSettingsDialog.ppi.toString()
                font.family: AppTheme.fontFamily
                font.pixelSize: 12
                validator: IntValidator { bottom: 72; top: 1200 }
                onEditingFinished: {
                    const v = parseInt(text)
                    if (!isNaN(v)) exportSettingsDialog.ppi = Math.max(72, Math.min(1200, v))
                }
            }

            // 操作
            Row {
                topPadding: 8
                spacing: 12
                Item { width: parent.parent.width - 44 - 220; height: 1 }
                AcrylicToolButton {
                    text: qsTr("取消")
                    onClicked: exportSettingsDialog.close()
                }
                AcrylicToolButton {
                    text: qsTr("导出")
                    glyph: "↓"
                    primary: true
                    onClicked: {
                        exportSettingsDialog.close()
                        exportPageDialog.open()
                    }
                }
            }
        }
    }

    // ---- 环境底 ----
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f0ece6" }
            GradientStop { position: 0.55; color: "#e6e2d9" }
            GradientStop { position: 1.0; color: "#dcd7cb" }
        }
    }

    // ---- 顶栏 ----
    AcrylicPanel {
        id: topbar
        anchors { left: parent.left; right: parent.right; top: parent.top; leftMargin: 18; rightMargin: 18; topMargin: 14 }
        height: 58

        Row {
            anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
            spacing: 12
            Text {
                text: qsTr("留素")
                color: AppTheme.ink
                font.family: AppTheme.fontFamily
                font.pixelSize: 17
                font.bold: true
                font.letterSpacing: 3.5
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: "LIUSU IMPRINT"
                color: AppTheme.ink3
                font.family: AppTheme.fontFamily
                font.pixelSize: 9
                font.bold: true
                font.letterSpacing: 2.2
                anchors.verticalCenter: parent.verticalCenter
            }
            Item { width: 6; height: 1 }
            // 导航
            Text {
                text: qsTr("01 主页")
                color: AppTheme.ink2
                font.family: AppTheme.fontFamily
                font.pixelSize: 13
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: editor.requestHome()
                }
            }
            Item {
                width: navEdit.width
                height: 26
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    id: navEdit
                    anchors.centerIn: parent
                    text: qsTr("02 编辑")
                    color: AppTheme.ink
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 13
                    font.bold: true
                }
                Rectangle {
                    anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom }
                    width: parent.width
                    height: 2
                    color: AppTheme.signal
                }
            }
            Text {
                text: qsTr("03 设置")
                color: AppTheme.ink3
                opacity: 0.55
                font.family: AppTheme.fontFamily
                font.pixelSize: 13
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Row {
            anchors { right: parent.right; rightMargin: 14; verticalCenter: parent.verticalCenter }
            spacing: 5
            AcrylicToolButton { text: qsTr("打开项目"); onClicked: openProjectDialog.open() }
            AcrylicToolButton { text: qsTr("保存项目"); onClicked: saveProjectDialog.open() }
            Rectangle { width: 1; height: 20; color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.12); anchors.verticalCenter: parent.verticalCenter }
            AcrylicToolButton { text: qsTr("批量导入"); glyph: "⇪"; onClicked: batchImportDialog.open() }
            AcrylicToolButton { text: qsTr("新建页"); glyph: "＋"; onClicked: app.addPage() }
            Rectangle { width: 1; height: 20; color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.12); anchors.verticalCenter: parent.verticalCenter }
            AcrylicToolButton { text: qsTr("导出本页"); glyph: "↓"; onClicked: exportSettingsDialog.open() }
            AcrylicToolButton { text: qsTr("导出全部"); glyph: "⇊"; onClicked: exportAllDialog.open() }
            Rectangle { width: 1; height: 20; color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.12); anchors.verticalCenter: parent.verticalCenter }
            AcrylicToolButton {
                glyph: "🗑"
                danger: true
                interactive: app.pageCount > 1
                onClicked: app.deleteCurrentPage()
            }
        }
    }

    // ---- 背景行 ----
    AcrylicPanel {
        id: bgRow
        anchors { left: parent.left; right: parent.right; top: topbar.bottom; leftMargin: 18; rightMargin: 18; topMargin: 12 }
        height: 52
        showScrews: false

        Row {
            anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
            spacing: 14
            Column {
                anchors.verticalCenter: parent.verticalCenter
                Text { text: "BACKGROUND"; color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 10; font.bold: true; font.letterSpacing: 2.2 }
                Text { text: qsTr("背景"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 13; font.bold: true }
            }
            // 纯色 / 纹理（纹理为未来方向，视觉上禁用）
            Row {
                anchors.verticalCenter: parent.verticalCenter
                Rectangle {
                    width: 56; height: 28
                    color: AppTheme.ink
                    Text { anchors.centerIn: parent; text: qsTr("纯色"); color: "#f4f1ea"; font.pixelSize: 12; font.bold: true; font.family: AppTheme.fontFamily }
                }
                Rectangle {
                    width: 56; height: 28
                    color: "transparent"
                    border.width: 1
                    border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.2)
                    Text { anchors.centerIn: parent; text: qsTr("纹理"); color: AppTheme.ink3; opacity: 0.6; font.pixelSize: 12; font.bold: true; font.family: AppTheme.fontFamily }
                }
            }
            // 色板
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 9
                Repeater {
                    model: ["#ffffff", "#f5f0e6", "#efe4d2", "#dce5dc", "#dde6f0", "#eadada", "#2f3338"]
                    delegate: Rectangle {
                        required property string modelData
                        width: 22; height: 22
                        color: modelData
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.8)
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.width: 1
                            border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.15)
                        }
                        Rectangle {
                            anchors { fill: parent; margins: -3 }
                            color: "transparent"
                            border.width: 2
                            border.color: AppTheme.signal
                            visible: app.backgroundHex === modelData
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: app.setBackground(modelData)
                        }
                    }
                }
                // 自定义颜色
                Rectangle {
                    width: 22; height: 22
                    radius: 0
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#c96a5a" }
                        GradientStop { position: 0.25; color: "#c9a05a" }
                        GradientStop { position: 0.5; color: "#8fae72" }
                        GradientStop { position: 0.75; color: "#6a93a8" }
                        GradientStop { position: 1.0; color: "#8a7aa8" }
                    }
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.8)
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: backgroundDialog.open()
                    }
                }
            }
        }

        Text {
            anchors { right: parent.right; rightMargin: 20; verticalCenter: parent.verticalCenter }
            text: qsTr("SELECT A SLOT → 旋转 / 镜像 / 更换 / 清除")
            color: AppTheme.ink3
            font.family: AppTheme.fontFamily
            font.pixelSize: 11
            font.letterSpacing: 1.2
        }
    }

    // ---- 工作台 ----
    Item {
        id: workbench
        anchors { left: parent.left; right: parent.right; top: bgRow.bottom; bottom: filmstrip.top; leftMargin: 18; rightMargin: 18; topMargin: 12; bottomMargin: 12 }

        // 台面
        Rectangle {
            anchors.fill: parent
            color: AppTheme.bench
            border.width: 1
            border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.12)
        }
        // 坐标纸网格
        Canvas {
            anchors.fill: parent
            opacity: 0.5
            onPaint: {
                const ctx = getContext("2d");
                ctx.reset();
                ctx.strokeStyle = "rgba(28,25,18,0.06)";
                ctx.lineWidth = 1;
                for (let x = 0; x < width; x += 38) {
                    ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, height); ctx.stroke();
                }
                for (let y = 0; y < height; y += 38) {
                    ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke();
                }
            }
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
        }
        // 点空白清空选择
        MouseArea {
            anchors.fill: parent
            onClicked: app.clearSelection()
        }

        // 相纸（有厚度的实体）
        Item {
            id: deskwrap
            width: paper.width
            height: paper.height
            anchors.centerIn: parent

            // 纸厚：底边与右侧露出的纸侧
            Rectangle {
                x: 0; y: paper.height
                width: paper.width; height: 7
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#d9d4c8" }
                    GradientStop { position: 1.0; color: "#b0aa9a" }
                }
            }
            Rectangle {
                x: paper.width; y: 0
                width: 7; height: paper.height
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: "#efece4" }
                    GradientStop { position: 1.0; color: "#c6c0b1" }
                }
            }

            Rectangle {
                id: paper
                width: 484
                height: 327
                color: "#ffffff"
                border.width: 1
                border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.10)
            }

            // 渲染结果（唯一真相 = PageRenderer 输出）
            // sourceSize 让图像提供器按显示尺寸推导 PPI 渲染，避免过采样。
            Image {
                anchors.fill: paper
                source: app.previewUrl
                sourceSize.width: paper.width
                sourceSize.height: paper.height
                cache: false
                fillMode: Image.Stretch
            }

            // 裁切角标
            Repeater {
                model: [
                    { cx: -19, cy: -19, flipX: 1, flipY: 1 },
                    { cx: paper.width, cy: -19, flipX: -1, flipY: 1 },
                    { cx: -19, cy: paper.height, flipX: 1, flipY: -1 },
                    { cx: paper.width, cy: paper.height, flipX: -1, flipY: -1 }
                ]
                delegate: Item {
                    required property var modelData
                    x: modelData.cx
                    y: modelData.cy
                    width: 18
                    height: 18
                    Rectangle {
                        width: parent.width * (modelData.flipX > 0 ? 1 : 0)
                        height: 1
                        color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.5)
                        anchors.right: modelData.flipX < 0 ? parent.right : undefined
                        anchors.bottom: modelData.flipY > 0 ? parent.bottom : undefined
                        anchors.top: modelData.flipY < 0 ? parent.top : undefined
                    }
                    Rectangle {
                        width: 1
                        height: parent.height * (modelData.flipY > 0 ? 1 : 0)
                        color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.5)
                        anchors.left: modelData.flipX > 0 ? parent.left : undefined
                        anchors.right: modelData.flipX < 0 ? parent.right : undefined
                        anchors.bottom: modelData.flipY > 0 ? parent.bottom : undefined
                        anchors.top: modelData.flipY < 0 ? parent.top : undefined
                    }
                }
            }

            // 槽位点击选择
            MouseArea {
                anchors.fill: paper
                cursorShape: Qt.PointingHandCursor
                onClicked: (mouse) => app.selectSlotAt(mouse.x / paper.width, mouse.y / paper.height)
            }

            // 选中描边
            readonly property var selRect: app.selectedSlotRect
            readonly property bool hasSelection: app.selectedSlot >= 0 && selRect["width"] !== undefined
            Rectangle {
                visible: deskwrap.hasSelection
                x: deskwrap.selRect.x * paper.width
                y: deskwrap.selRect.y * paper.height
                width: deskwrap.selRect.width * paper.width
                height: deskwrap.selRect.height * paper.height
                color: "transparent"
                border.width: 2
                border.color: AppTheme.signal
            }

            // 槽位工具板（亚克力小块，浮在纸前）
            AcrylicPanel {
                id: slotTools
                visible: deskwrap.hasSelection
                width: 5 * 40 + 10
                height: 42
                cornerRadius: 2
                x: Math.max(0, Math.min(paper.width - width,
                          deskwrap.selRect.x * paper.width + deskwrap.selRect.width * paper.width / 2 - width / 2))
                y: Math.min(paper.height + 14,
                          deskwrap.selRect.y * paper.height + deskwrap.selRect.height * paper.height + 12)

                Row {
                    anchors.centerIn: parent
                    spacing: 0
                    Repeater {
                        model: [
                            { glyph: "⟳", tip: qsTr("旋转") },
                            { glyph: "⇋", tip: qsTr("镜像") },
                            { glyph: "▢", tip: qsTr("完整放入 / 铺满裁切") },
                            { glyph: "⇪", tip: qsTr("更换照片") },
                            { glyph: "✕", tip: qsTr("清除") }
                        ]
                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            width: 40
                            height: 34
                            color: toolHover.containsMouse ? AppTheme.signal : "transparent"
                            Text {
                                anchors.centerIn: parent
                                text: modelData.glyph
                                color: toolHover.containsMouse ? "#ffffff" : AppTheme.ink
                                font.pixelSize: 14
                            }
                            MouseArea {
                                id: toolHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    const slot = app.selectedSlot
                                    if (index === 0) app.rotateSlot(slot)
                                    else if (index === 1) app.mirrorSlot(slot)
                                    else if (index === 2) app.toggleFillMode(slot)
                                    else if (index === 3) { replaceDialog.slotIndex = slot; replaceDialog.open() }
                                    else app.clearSlot(slot)
                                }
                            }
                        }
                    }
                }
            }

            // 页面标签
            Row {
                anchors { left: paper.left; bottom: paper.top; bottomMargin: 8 }
                spacing: 8
                Rectangle {
                    width: 18; height: 3
                    color: AppTheme.signal
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: app.pageLabel + " · " + app.currentSlotCount + qsTr(" 槽位 · 148×100 MM")
                    color: AppTheme.ink2
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 10
                    font.bold: true
                    font.letterSpacing: 1.8
                }
            }
        }
    }

    // ---- 胶片栏（页队列）----
    AcrylicPanel {
        id: filmstrip
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; leftMargin: 18; rightMargin: 18; bottomMargin: 16 }
        height: 90
        showScrews: false

        Row {
            anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
            spacing: 16
            Column {
                anchors.verticalCenter: parent.verticalCenter
                Text { text: "FILMSTRIP"; color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 10; font.bold: true; font.letterSpacing: 2.2 }
                Text { text: qsTr("页队列"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 13; font.bold: true }
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12
                Repeater {
                    model: app.pageCount
                    delegate: Rectangle {
                        required property int index
                        width: 86
                        height: 58
                        color: "#ffffff"
                        border.width: app.currentPageIndex === index ? 0 : 1
                        border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.15)
                        Rectangle {
                            anchors { fill: parent; margins: -2 }
                            color: "transparent"
                            border.width: 2
                            border.color: AppTheme.signal
                            visible: app.currentPageIndex === index
                        }
                        Image {
                            anchors.fill: parent
                            anchors.margins: 3
                            // 逐页修订 URL：编辑其它页不会让本页缩略图重渲染。
                            source: app.pageThumbnailUrl(index)
                            sourceSize.width: 172
                            sourceSize.height: 116
                            cache: false
                            fillMode: Image.PreserveAspectFit
                        }
                        Rectangle {
                            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                            height: 13
                            color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.78)
                            Text {
                                anchors.centerIn: parent
                                text: "PG-" + (index + 1).toString().padStart(3, "0")
                                color: "#efece4"
                                font.family: AppTheme.fontFamily
                                font.pixelSize: 8
                                font.letterSpacing: 1.5
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: app.setCurrentPage(index)
                        }
                    }
                }
                // 新建页
                Rectangle {
                    width: 86
                    height: 58
                    color: addHover.containsMouse ? AppTheme.signalSoft : "transparent"
                    border.width: 1
                    border.color: addHover.containsMouse ? AppTheme.signal : Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.28)
                    Text {
                        anchors.centerIn: parent
                        text: "＋"
                        color: addHover.containsMouse ? AppTheme.signalDeep : AppTheme.ink3
                        font.pixelSize: 17
                    }
                    MouseArea {
                        id: addHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: app.addPage()
                    }
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("连续排多页 · 整队导出")
                color: AppTheme.ink3
                font.family: AppTheme.fontFamily
                font.pixelSize: 11
                font.letterSpacing: 1.2
            }
        }
    }

    // ---- 状态吐司（亚克力 + 左琥珀条）----
    AcrylicPanel {
        id: toast
        visible: app.statusMessage.length > 0
        anchors { right: parent.right; rightMargin: 36; bottom: filmstrip.top; bottomMargin: 20 }
        height: 44
        width: toastRow.width + 44
        cornerRadius: 2
        showScrews: false
        opacity: visible ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
        }
        Rectangle {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            width: 3
            color: AppTheme.signal
        }
        Row {
            id: toastRow
            anchors.centerIn: parent
            spacing: 10
            Rectangle {
                width: 16; height: 16; radius: 8
                color: AppTheme.signal
                anchors.verticalCenter: parent.verticalCenter
                Text { anchors.centerIn: parent; text: "✓"; color: "#ffffff"; font.pixelSize: 10; font.bold: true }
            }
            Text {
                text: app.statusMessage
                color: AppTheme.ink2
                font.family: AppTheme.fontFamily
                font.pixelSize: 12
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        Timer {
            interval: 4200
            running: toast.visible
            onTriggered: app.clearStatus()
        }
    }

    // 点击工作台之外的区域也清空选择（顶栏/背景行/胶片栏）
    MouseArea {
        anchors.fill: parent
        z: -1
        onClicked: app.clearSelection()
    }
}
