import QtQuick
import LiuSu
import QtQuick.Controls
import QtQuick.Dialogs

// 编辑页（LR 三栏 + 3D 深度 + 动效）：
//   左轨：背景色板 + 布局读数 | 中央：工作台 + 相纸 + 浮动槽位工具 | 右栏：页面读数 + 导出
//   槽位工具贴近选中槽位弹出，不在远处工具栏。
// UI 只做展示与交互；所有几何与像素真相在 domain / render 层。
Item {
    id: editor

    signal requestHome()

    // ==================== 对话框 ====================
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
        defaultSuffix: rightPanel.expJpeg ? "jpg" : "png"
        nameFilters: rightPanel.expJpeg ? [qsTr("JPEG (*.jpg)")] : [qsTr("PNG (*.png)")]
        onAccepted: app.exportCurrentPage(selectedFile, rightPanel.expPpi, rightPanel.expJpeg, 95)
    }
    FolderDialog {
        id: exportAllDialog
        title: qsTr("选择导出目录")
        onAccepted: app.exportAllPages(selectedFolder, rightPanel.expPpi, rightPanel.expJpeg, 95)
    }

    // ==================== 入场动效 ====================
    // 用 ParallelAnimation 同时动 opacity 和位移，避免 onYChanged 卡死 opacity=0
    Timer {
        id: entranceTimer
        interval: 1
        running: true
        onTriggered: entranceAnim.start()
    }
    ParallelAnimation {
        id: entranceAnim
        // 环境底
        NumberAnimation { target: bgRect; property: "opacity"; to: 1; duration: 400 }
        // 顶栏从上滑入
        ParallelAnimation {
            NumberAnimation { target: topbarWrap; property: "opacity"; to: 1; duration: 450; easing.type: Easing.OutCubic }
            NumberAnimation { target: topbarWrap; property: "y"; from: topbarWrap.y - 30; to: topbarWrap.y; duration: 450; easing.type: Easing.OutCubic }
        }
        // 左轨从左滑入
        ParallelAnimation {
            NumberAnimation { target: leftRailWrap; property: "opacity"; to: 1; duration: 450; easing.type: Easing.OutCubic }
            NumberAnimation { target: leftRailWrap; property: "x"; from: leftRailWrap.x - 40; to: leftRailWrap.x; duration: 450; easing.type: Easing.OutCubic }
        }
        // 右栏从右滑入
        ParallelAnimation {
            NumberAnimation { target: rightPanelWrap; property: "opacity"; to: 1; duration: 450; easing.type: Easing.OutCubic }
            NumberAnimation { target: rightPanelWrap; property: "x"; from: rightPanelWrap.x + 40; to: rightPanelWrap.x; duration: 450; easing.type: Easing.OutCubic }
        }
        // 工作台从下滑入
        ParallelAnimation {
            NumberAnimation { target: workbenchWrap; property: "opacity"; to: 1; duration: 500; easing.type: Easing.OutCubic }
            NumberAnimation { target: workbenchWrap; property: "y"; from: workbenchWrap.y + 20; to: workbenchWrap.y; duration: 500; easing.type: Easing.OutCubic }
        }
        // 胶片栏从下滑入
        ParallelAnimation {
            NumberAnimation { target: filmstripWrap; property: "opacity"; to: 1; duration: 450; easing.type: Easing.OutCubic }
            NumberAnimation { target: filmstripWrap; property: "y"; from: filmstripWrap.y + 30; to: filmstripWrap.y; duration: 450; easing.type: Easing.OutCubic }
        }
    }

    // ==================== 环境底 ====================
    Rectangle {
        id: bgRect
        anchors.fill: parent
        opacity: 0
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f0ece6" }
            GradientStop { position: 0.55; color: "#e6e2d9" }
            GradientStop { position: 1.0; color: "#dcd7cb" }
        }
    }
    Canvas {
        opacity: bgRect.opacity
        x: -80; y: -120; width: 640; height: 420
        onPaint: {
            const ctx = getContext("2d"); ctx.reset();
            const g = ctx.createRadialGradient(width/2, height/2, 0, width/2, height/2, width/2);
            g.addColorStop(0, "rgba(255,255,255,0.55)");
            g.addColorStop(0.6, "rgba(255,255,255,0.20)");
            g.addColorStop(1, "rgba(255,255,255,0)");
            ctx.fillStyle = g; ctx.fillRect(0, 0, width, height);
        }
        onWidthChanged: requestPaint(); onHeightChanged: requestPaint()
    }
    Canvas {
        opacity: bgRect.opacity
        x: parent.width - 440; y: -60; width: 520; height: 380
        onPaint: {
            const ctx = getContext("2d"); ctx.reset();
            const g = ctx.createRadialGradient(width/2, height/2, 0, width/2, height/2, width/2);
            g.addColorStop(0, "rgba(217,142,43,0.10)");
            g.addColorStop(0.6, "rgba(217,142,43,0.04)");
            g.addColorStop(1, "rgba(217,142,43,0)");
            ctx.fillStyle = g; ctx.fillRect(0, 0, width, height);
        }
        onWidthChanged: requestPaint(); onHeightChanged: requestPaint()
    }

    // ==================== 顶栏 ====================
    Item {
        id: topbarWrap
        anchors { left: parent.left; right: parent.right; top: parent.top; leftMargin: 18; rightMargin: 18; topMargin: 12 }
        height: 50
        opacity: 0

        AcrylicPanel {
            anchors.fill: parent
            showScrews: false

            // 品牌 + 导航
            Row {
                anchors { left: parent.left; leftMargin: 18; verticalCenter: parent.verticalCenter }
                spacing: 10
                Text {
                    text: qsTr("留素"); color: AppTheme.ink
                    font.family: AppTheme.fontFamily; font.pixelSize: 16; font.bold: true; font.letterSpacing: 3
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "LIUSU IMPRINT"; color: AppTheme.ink3
                    font.family: AppTheme.fontFamily; font.pixelSize: 8; font.bold: true; font.letterSpacing: 2
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: 6; height: 1 }
                Text {
                    text: qsTr("01 主页"); color: AppTheme.ink2
                    font.family: AppTheme.fontFamily; font.pixelSize: 12; font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: editor.requestHome() }
                }
                Item {
                    width: navE.width; height: 24; anchors.verticalCenter: parent.verticalCenter
                    Text {
                        id: navE; anchors.centerIn: parent
                        text: qsTr("02 编辑"); color: AppTheme.ink
                        font.family: AppTheme.fontFamily; font.pixelSize: 12; font.bold: true
                    }
                    Rectangle {
                        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom }
                        width: parent.width; height: 2; color: AppTheme.signal
                    }
                }
            }

            // 工具区：文件组 | 内容组
            Row {
                anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
                spacing: 6

                // 文件组
                Row {
                    spacing: 2
                    anchors.verticalCenter: parent.verticalCenter
                    AcrylicToolButton { text: qsTr("打开"); onClicked: openProjectDialog.open() }
                    AcrylicToolButton { text: qsTr("保存"); onClicked: saveProjectDialog.open() }
                }
                Rectangle { width: 1; height: 20; color: AppTheme.line; anchors.verticalCenter: parent.verticalCenter }
                // 内容组
                Row {
                    spacing: 2
                    anchors.verticalCenter: parent.verticalCenter
                    AcrylicToolButton { text: qsTr("导入"); glyph: "⇪"; onClicked: batchImportDialog.open() }
                    AcrylicToolButton { text: qsTr("新建页"); glyph: "＋"; onClicked: app.addPage() }
                }
            }
        }
    }

    // ==================== 左轨：背景 + 布局 ====================
    Item {
        id: leftRailWrap
        anchors { left: parent.left; top: topbarWrap.bottom; bottom: filmstripWrap.top; leftMargin: 18; topMargin: 12; bottomMargin: 12 }
        width: 160
        opacity: 0

        AcrylicPanel {
            id: leftRail
            anchors.fill: parent

            // 悬停 3D 浮起
            scale: railHover.containsMouse ? 1.008 : 1.0
            Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
            MouseArea {
                id: railHover
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
                z: -1
            }

            Column {
                anchors { fill: parent; margins: 14 }
                spacing: 12

                // 背景色板
                Text {
                    text: "BACKGROUND · 背景"; color: AppTheme.ink
                    font.family: AppTheme.fontFamily; font.pixelSize: 11; font.bold: true; font.letterSpacing: 1.2
                }
                Grid {
                    columns: 4
                    spacing: 5
                    Repeater {
                        model: ["#ffffff", "#f5f0e6", "#efe4d2", "#dce5dc", "#dde6f0", "#eadada", "#2f3338", "#c8b89a"]
                        delegate: Item {
                            required property string modelData
                            width: 30; height: 30
                            // 悬停微浮
                            scale: swatchHover.containsMouse ? 1.15 : 1.0
                            Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                            z: swatchHover.containsMouse ? 2 : 0

                            Rectangle {
                                anchors.fill: parent
                                color: modelData
                                border.width: 1; border.color: Qt.rgba(1, 1, 1, 0.8)
                            }
                            Rectangle {
                                anchors.fill: parent
                                color: "transparent"
                                border.width: 1; border.color: AppTheme.line
                            }
                            // 选中光晕
                            Rectangle {
                                anchors.fill: parent; anchors.margins: -3
                                color: "transparent"
                                border.width: 2; border.color: AppTheme.signal
                                visible: app.backgroundHex === modelData
                            }
                            Rectangle {
                                anchors { left: parent.left; right: parent.right; top: parent.top; leftMargin: 1; rightMargin: 1 }
                                height: 1; color: Qt.rgba(1, 1, 1, 0.6)
                            }
                            MouseArea {
                                id: swatchHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: app.setBackground(modelData)
                            }
                        }
                    }
                }
                Rectangle {
                    width: 30; height: 30
                    scale: customSwatchHover.containsMouse ? 1.15 : 1.0
                    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#c96a5a" }
                        GradientStop { position: 0.25; color: "#c9a05a" }
                        GradientStop { position: 0.5; color: "#8fae72" }
                        GradientStop { position: 0.75; color: "#6a93a8" }
                        GradientStop { position: 1.0; color: "#8a7aa8" }
                    }
                    border.width: 1; border.color: Qt.rgba(1, 1, 1, 0.8)
                    MouseArea {
                        id: customSwatchHover
                        anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: backgroundDialog.open()
                    }
                }

                Rectangle { width: parent.width; height: 1; color: AppTheme.line }

                // 布局读数
                Text {
                    text: "LAYOUT · 布局"; color: AppTheme.ink
                    font.family: AppTheme.fontFamily; font.pixelSize: 11; font.bold: true; font.letterSpacing: 1.2
                }
                Text {
                    text: qsTr("槽位 %1 个 · %2×%3 MM")
                            .arg(app.currentSlotCount)
                            .arg(app.pageWidthMm).arg(app.pageHeightMm)
                    color: AppTheme.ink2
                    font.family: AppTheme.fontFamily; font.pixelSize: 11
                }

                Rectangle { width: parent.width; height: 1; color: AppTheme.line }

                // 快捷操作（从顶栏迁移的低频功能）
                Text {
                    text: "PAGES · 页管理"; color: AppTheme.ink
                    font.family: AppTheme.fontFamily; font.pixelSize: 11; font.bold: true; font.letterSpacing: 1.2
                }
                Rectangle {
                    width: parent.width; height: 30; radius: 2
                    color: delHover.containsMouse ? Qt.rgba(179/255, 64/255, 46/255, 0.10) : "transparent"
                    border.width: 1
                    border.color: delHover.containsMouse ? AppTheme.danger : AppTheme.line
                    opacity: app.pageCount > 1 ? 1.0 : 0.4
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("✕ 删除当前页")
                        color: delHover.containsMouse ? AppTheme.danger : AppTheme.ink2
                        font.family: AppTheme.fontFamily; font.pixelSize: 10; font.bold: true
                    }
                    MouseArea {
                        id: delHover
                        anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        enabled: app.pageCount > 1
                        onClicked: app.deleteCurrentPage()
                    }
                }
            }
        }
    }

    // ==================== 中央工作台 ====================
    Item {
        id: workbenchWrap
        anchors { left: leftRailWrap.right; right: rightPanelWrap.left; top: topbarWrap.bottom; bottom: filmstripWrap.top; leftMargin: 12; rightMargin: 12; topMargin: 12; bottomMargin: 12 }
        opacity: 0

        Item {
            id: workbench
            anchors.fill: parent

            // 台面（下沉暗面）
            Rectangle {
                anchors.fill: parent
                color: AppTheme.bench
                border.width: 1; border.color: Qt.rgba(28/255, 25/255, 18/255, 0.16)
            }
            // 内阴影
            Canvas {
                anchors.fill: parent; opacity: 0.8
                onPaint: {
                    const ctx = getContext("2d"); ctx.reset();
                    let g = ctx.createLinearGradient(0, 0, 0, 40);
                    g.addColorStop(0, "rgba(28,25,18,0.12)"); g.addColorStop(1, "rgba(28,25,18,0)");
                    ctx.fillStyle = g; ctx.fillRect(0, 0, width, 40);
                    g = ctx.createLinearGradient(0, height, 0, height-40);
                    g.addColorStop(0, "rgba(28,25,18,0.10)"); g.addColorStop(1, "rgba(28,25,18,0)");
                    ctx.fillStyle = g; ctx.fillRect(0, height-40, width, 40);
                    g = ctx.createLinearGradient(0, 0, 40, 0);
                    g.addColorStop(0, "rgba(28,25,18,0.08)"); g.addColorStop(1, "rgba(28,25,18,0)");
                    ctx.fillStyle = g; ctx.fillRect(0, 0, 40, height);
                    g = ctx.createLinearGradient(width, 0, width-40, 0);
                    g.addColorStop(0, "rgba(28,25,18,0.08)"); g.addColorStop(1, "rgba(28,25,18,0)");
                    ctx.fillStyle = g; ctx.fillRect(width-40, 0, 40, height);
                }
                onWidthChanged: requestPaint(); onHeightChanged: requestPaint()
            }
            // 网格
            Canvas {
                anchors.fill: parent; opacity: 0.5
                onPaint: {
                    const ctx = getContext("2d"); ctx.reset();
                    ctx.strokeStyle = "rgba(28,25,18,0.06)"; ctx.lineWidth = 1;
                    for (let x = 0; x < width; x += 38) { ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, height); ctx.stroke(); }
                    for (let y = 0; y < height; y += 38) { ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke(); }
                }
                onWidthChanged: requestPaint(); onHeightChanged: requestPaint()
            }
            // 四角 L 标
            Repeater {
                model: [
                    { cx: 18, cy: 18, sx: 1, sy: 1 },
                    { cx: workbench.width - 18, cy: 18, sx: -1, sy: 1 },
                    { cx: 18, cy: workbench.height - 18, sx: 1, sy: -1 },
                    { cx: workbench.width - 18, cy: workbench.height - 18, sx: -1, sy: -1 }
                ]
                delegate: Item {
                    required property var modelData
                    x: modelData.cx - (modelData.sx > 0 ? 0 : 14)
                    y: modelData.cy - (modelData.sy > 0 ? 0 : 14)
                    width: 14; height: 14
                    Rectangle {
                        width: 14; height: 1.5
                        color: Qt.rgba(28/255, 25/255, 18/255, 0.30)
                        anchors.top: modelData.sy > 0 ? parent.top : undefined
                        anchors.bottom: modelData.sy < 0 ? parent.bottom : undefined
                    }
                    Rectangle {
                        width: 1.5; height: 14
                        color: Qt.rgba(28/255, 25/255, 18/255, 0.30)
                        anchors.left: modelData.sx > 0 ? parent.left : undefined
                        anchors.right: modelData.sx < 0 ? parent.right : undefined
                    }
                }
            }
            // 台面读数
            Text {
                x: 24; y: 22
                text: "BENCH · " + app.pageLabel + " · " + app.pageWidthMm + "×" + app.pageHeightMm + " MM"
                color: AppTheme.ink3
                font.family: AppTheme.fontFamily; font.pixelSize: 8; font.letterSpacing: 1.5; font.bold: true
            }
            // 微刻度
            Canvas {
                x: 24; y: 48; width: 8; height: workbench.height - 96
                onPaint: {
                    const ctx = getContext("2d"); ctx.reset();
                    ctx.strokeStyle = "rgba(28,25,18,0.15)"; ctx.lineWidth = 1;
                    for (let y = 0; y < height; y += 14) {
                        const len = (y % 56 === 0) ? 8 : 4;
                        ctx.beginPath(); ctx.moveTo(8-len, y); ctx.lineTo(8, y); ctx.stroke();
                    }
                }
                onWidthChanged: requestPaint(); onHeightChanged: requestPaint()
            }

            MouseArea {
                anchors.fill: parent
                onClicked: app.clearSelection()
            }

            // ---- 相纸（真 3D 旋转体：鼠标跟随倾斜 + 可见侧壁）----
            Item {
                id: deskwrap
                width: paper.width
                height: paper.height
                anchors.centerIn: parent

                // 3D 倾斜状态（鼠标驱动）
                property real tiltX: 0
                property real tiltY: 0
                property bool hovering: paperHover.containsMouse

                // 3D 变换：双轴旋转（对照 CoreKeeper 层叠玻璃版）
                transform: [
                    Rotation {
                        origin.x: paper.width / 2; origin.y: paper.height / 2
                        axis { x: 1; y: 0; z: 0 }
                        angle: deskwrap.tiltX
                    },
                    Rotation {
                        origin.x: paper.width / 2; origin.y: paper.height / 2
                        axis { x: 0; y: 1; z: 0 }
                        angle: deskwrap.tiltY
                    }
                ]

                // 鼠标跟踪倾斜（±6°，弹性回正）
                Behavior on tiltX { NumberAnimation { duration: 350; easing.type: Easing.OutCubic } }
                Behavior on tiltY { NumberAnimation { duration: 350; easing.type: Easing.OutCubic } }

                MouseArea {
                    id: paperHover
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.NoButton
                    onPositionChanged: (mouse) => {
                        deskwrap.tiltY = (mouse.x / paper.width - 0.5) * 12
                        deskwrap.tiltX = -(mouse.y / paper.height - 0.5) * 12
                    }
                    onExited: {
                        deskwrap.tiltX = 0
                        deskwrap.tiltY = 0
                    }
                }

                // 极轻单层投影
                Rectangle {
                    x: 3; y: 5
                    width: paper.width; height: paper.height
                    radius: 2
                    color: Qt.rgba(28/255, 25/255, 18/255, 0.12)
                    z: -10
                }

                // 底侧壁（rotateX 90° → 可见纸厚面）
                Rectangle {
                    x: 0; y: paper.height
                    width: paper.width; height: 10
                    transformOrigin: Item.Top
                    transform: Rotation {
                        origin.x: paper.width / 2; origin.y: 0
                        axis { x: 1; y: 0; z: 0 }
                        angle: 90 - deskwrap.tiltX * 0.3
                    }
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#ddd8cc" }
                        GradientStop { position: 1.0; color: "#b0aa98" }
                    }
                }
                // 右侧壁（rotateY -90° → 可见纸厚面）
                Rectangle {
                    x: paper.width; y: 0
                    width: 10; height: paper.height
                    transformOrigin: Item.Left
                    transform: Rotation {
                        origin.x: 0; origin.y: paper.height / 2
                        axis { x: 0; y: 1; z: 0 }
                        angle: -90 + deskwrap.tiltY * 0.3
                    }
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#e4e0d6" }
                        GradientStop { position: 1.0; color: "#b8b2a0" }
                    }
                }

                // 主面（照片纸）
                Rectangle {
                    id: paper
                    x: 0; y: 0
                    width: 484; height: 327
                    color: "#ffffff"
                    radius: 1
                    border.width: 1
                    border.color: Qt.rgba(28/255, 25/255, 18/255, 0.06)

                    // 顶部微光
                    Rectangle {
                        anchors { left: parent.left; right: parent.right; top: parent.top; leftMargin: 1; rightMargin: 1 }
                        height: 1
                        color: Qt.rgba(1, 1, 1, 0.70)
                    }

                    // 渲染结果
                    Image {
                        anchors.fill: parent
                        source: app.previewUrl
                        sourceSize.width: paper.width
                        sourceSize.height: paper.height
                        cache: false
                        fillMode: Image.Stretch
                    }
                }

                // 裁切角标
                Repeater {
                    model: [
                        { cx: -16, cy: -16, fx: 1, fy: 1 },
                        { cx: paper.width, cy: -16, fx: -1, fy: 1 },
                        { cx: -16, cy: paper.height, fx: 1, fy: -1 },
                        { cx: paper.width, cy: paper.height, fx: -1, fy: -1 }
                    ]
                    delegate: Item {
                        required property var modelData
                        x: modelData.cx; y: modelData.cy
                        width: 16; height: 16
                        Rectangle {
                            width: modelData.fx > 0 ? 16 : 0; height: 1.5
                            color: Qt.rgba(28/255, 25/255, 18/255, 0.40)
                            anchors.bottom: modelData.fy > 0 ? parent.bottom : undefined
                            anchors.top: modelData.fy < 0 ? parent.top : undefined
                        }
                        Rectangle {
                            width: 1.5; height: modelData.fy > 0 ? 16 : 0
                            color: Qt.rgba(28/255, 25/255, 18/255, 0.40)
                            anchors.left: modelData.fx > 0 ? parent.left : undefined
                            anchors.right: modelData.fx < 0 ? parent.right : undefined
                        }
                    }
                }

                // 槽位选择
                MouseArea {
                    anchors.fill: paper
                    cursorShape: Qt.PointingHandCursor
                    onClicked: (mouse) => app.selectSlotAt(mouse.x / paper.width, mouse.y / paper.height)
                }

                // 选中状态
                readonly property var selRect: app.selectedSlotRect
                readonly property bool hasSel: app.selectedSlot >= 0 && selRect["width"] !== undefined

                // 琥珀光晕
                Rectangle {
                    visible: deskwrap.hasSel
                    x: paper.x + deskwrap.selRect.x * paper.width - 5
                    y: paper.y + deskwrap.selRect.y * paper.height - 5
                    width: deskwrap.selRect.width * paper.width + 10
                    height: deskwrap.selRect.height * paper.height + 10
                    color: "transparent"
                    border.width: 1
                    border.color: Qt.rgba(217/255, 142/255, 43/255, 0.35)
                    SequentialAnimation on opacity {
                        running: deskwrap.hasSel
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 0.55; duration: 1300; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 0.55; to: 1.0; duration: 1300; easing.type: Easing.InOutSine }
                    }
                }
                // 选中框
                Rectangle {
                    visible: deskwrap.hasSel
                    x: paper.x + deskwrap.selRect.x * paper.width
                    y: paper.y + deskwrap.selRect.y * paper.height
                    width: deskwrap.selRect.width * paper.width
                    height: deskwrap.selRect.height * paper.height
                    color: "transparent"
                    border.width: 2; border.color: AppTheme.signal
                }
                // 四角琥珀点
                Repeater {
                    model: deskwrap.hasSel ? 4 : 0
                    delegate: Rectangle {
                        required property int index
                        x: paper.x + deskwrap.selRect.x * paper.width + (index % 2) * deskwrap.selRect.width * paper.width - 3
                        y: paper.y + deskwrap.selRect.y * paper.height + Math.floor(index / 2) * deskwrap.selRect.height * paper.height - 3
                        width: 6; height: 6; radius: 3; color: AppTheme.signal
                    }
                }

                // 页面标签
                Row {
                    anchors { left: paper.left; bottom: paper.top; bottomMargin: 8 }
                    spacing: 8
                    Rectangle {
                        width: 18; height: 3; color: AppTheme.signal
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: app.pageLabel + " · " + app.currentSlotCount + qsTr(" 槽位")
                        color: AppTheme.ink2
                        font.family: AppTheme.fontFamily; font.pixelSize: 10; font.bold: true; font.letterSpacing: 1.8
                    }
                }
                Text {
                    anchors { left: paper.left; top: paper.bottom; topMargin: 14 }
                    text: app.pageWidthMm + " MM"
                    color: AppTheme.ink3
                    font.family: AppTheme.fontFamily; font.pixelSize: 8; font.letterSpacing: 1.5; font.bold: true
                }
            }

            // ---- 槽位工具条（固定在相纸下方）----
            AcrylicPanel {
                id: slotToolsWrap
                visible: deskwrap.hasSel
                anchors.horizontalCenter: deskwrap.horizontalCenter
                anchors.top: deskwrap.bottom
                anchors.topMargin: 32
                width: 5 * 58 + 20
                height: 44
                showScrews: false
                // 入场动效
                opacity: visible ? 1 : 0
                y: visible ? 0 : 8
                Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
                Behavior on y { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

                Row {
                    anchors.centerIn: parent
                    spacing: 2
                    Repeater {
                        model: [
                            { glyph: "⟳", label: qsTr("旋转") },
                            { glyph: "⇋", label: qsTr("镜像") },
                            { glyph: "▢", label: qsTr("填充") },
                            { glyph: "⇪", label: qsTr("更换") },
                            { glyph: "✕", label: qsTr("清除") }
                        ]
                        delegate: Rectangle {
                            required property var modelData
                            required property int index
                            width: 58; height: 36
                            radius: 2
                            y: stHover.containsMouse ? -1 : 0
                            Behavior on y { NumberAnimation { duration: 120 } }

                            color: stHover.containsMouse
                                ? (index === 4 ? Qt.rgba(179/255, 64/255, 46/255, 0.12) : AppTheme.signalSoft)
                                : "transparent"

                            Row {
                                anchors.centerIn: parent
                                spacing: 4
                                Text {
                                    text: modelData.glyph
                                    color: stHover.containsMouse
                                        ? (index === 4 ? AppTheme.danger : AppTheme.signalDeep)
                                        : AppTheme.ink2
                                    font.pixelSize: 13; font.bold: true
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: modelData.label
                                    color: stHover.containsMouse
                                        ? (index === 4 ? AppTheme.danger : AppTheme.ink)
                                        : AppTheme.ink2
                                    font.family: AppTheme.fontFamily; font.pixelSize: 10; font.bold: true
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                            MouseArea {
                                id: stHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    const s = app.selectedSlot
                                    if (index === 0) app.rotateSlot(s)
                                    else if (index === 1) app.mirrorSlot(s)
                                    else if (index === 2) app.toggleFillMode(s)
                                    else if (index === 3) { replaceDialog.slotIndex = s; replaceDialog.open() }
                                    else app.clearSlot(s)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ==================== 右栏 ====================
    Item {
        id: rightPanelWrap
        anchors { right: parent.right; top: topbarWrap.bottom; bottom: filmstripWrap.top; rightMargin: 18; topMargin: 12; bottomMargin: 12 }
        width: 210
        opacity: 0

        AcrylicPanel {
            id: rightPanel
            anchors.fill: parent
            glassOpacity: 0.95   // 右栏略实，信息更清晰

            // 导出参数
            property int expPpi: 300
            property bool expJpeg: true

            // 上部信息（顶部对齐）
            Column {
                anchors { left: parent.left; right: parent.right; top: parent.top; margins: 16 }
                spacing: 0

                // ---- 页面信息 ----
                Text {
                    text: "PAGE · 页面"; color: AppTheme.ink3
                    font.family: AppTheme.fontFamily; font.pixelSize: 9; font.bold: true; font.letterSpacing: 2
                }
                Item { width: 1; height: 10 }
                Row {
                    spacing: 20
                    Column {
                        spacing: 2
                        Text { text: app.pageLabel.replace("PG-", ""); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 28; font.bold: true; font.letterSpacing: 1 }
                        Text { text: "PAGE"; color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 8; font.letterSpacing: 1.5; font.bold: true }
                    }
                    Column {
                        spacing: 2
                        Text { text: "" + app.currentSlotCount; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 28; font.bold: true }
                        Text { text: "SLOTS"; color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 8; font.letterSpacing: 1.5; font.bold: true }
                    }
                }
                Item { width: 1; height: 8 }
                Text {
                    text: app.pageWidthMm + " × " + app.pageHeightMm + " MM"
                    color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 12; font.bold: true
                }
                Item { width: 1; height: 6 }
                Row {
                    spacing: 6
                    Text { text: "BG"; color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 9; font.letterSpacing: 1.5; font.bold: true }
                    Rectangle {
                        width: 14; height: 14; y: -2
                        color: app.backgroundHex
                        border.width: 1; border.color: AppTheme.line
                    }
                    Text { text: app.backgroundHex; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
                }

                Item { width: 1; height: 16 }
                Rectangle { width: parent.width; height: 1; color: AppTheme.line }
                Item { width: 1; height: 16 }

                // ---- 输出参数 ----
                Text {
                    text: "OUTPUT · 输出"; color: AppTheme.ink3
                    font.family: AppTheme.fontFamily; font.pixelSize: 9; font.bold: true; font.letterSpacing: 2
                }
                Item { width: 1; height: 12 }

                Text { text: "PPI"; color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 9; font.letterSpacing: 1.5; font.bold: true }
                Item { width: 1; height: 6 }
                Row {
                    spacing: 4
                    Repeater {
                        model: [300, 600]
                        delegate: Rectangle {
                            required property int modelData
                            width: 58; height: 30; radius: 3
                            color: rightPanel.expPpi === modelData ? AppTheme.ink : "transparent"
                            border.width: 1; border.color: rightPanel.expPpi === modelData ? AppTheme.ink : AppTheme.line
                            Text {
                                anchors.centerIn: parent
                                text: "" + modelData
                                color: rightPanel.expPpi === modelData ? "#f4f1ea" : AppTheme.ink2
                                font.family: AppTheme.fontFamily; font.pixelSize: 12; font.bold: true
                            }
                            MouseArea {
                                anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onClicked: rightPanel.expPpi = modelData
                            }
                        }
                    }
                }

                Item { width: 1; height: 12 }
                Text { text: qsTr("格式"); color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 9; font.letterSpacing: 1.5; font.bold: true }
                Item { width: 1; height: 6 }
                Row {
                    spacing: 0
                    Repeater {
                        model: [{ label: "JPEG", v: true }, { label: "PNG", v: false }]
                        delegate: Rectangle {
                            required property var modelData
                            width: 60; height: 30; radius: 3
                            color: rightPanel.expJpeg === modelData.v ? AppTheme.ink : "transparent"
                            border.width: 1; border.color: rightPanel.expJpeg === modelData.v ? AppTheme.ink : AppTheme.line
                            Text {
                                anchors.centerIn: parent; text: modelData.label
                                color: rightPanel.expJpeg === modelData.v ? "#f4f1ea" : AppTheme.ink2
                                font.family: AppTheme.fontFamily; font.pixelSize: 11; font.bold: true
                            }
                            MouseArea {
                                anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onClicked: rightPanel.expJpeg = modelData.v
                            }
                        }
                    }
                }

                Item { width: 1; height: 10 }
                Text {
                    text: qsTr("约 %1 × %2 PX")
                            .arg(app.pagePixelWidth(rightPanel.expPpi))
                            .arg(app.pagePixelHeight(rightPanel.expPpi))
                    color: AppTheme.ink2
                    font.family: AppTheme.fontFamily; font.pixelSize: 11
                }
            }

            // ---- 导出操作（锚定底部）----
            Column {
                anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 16 }
                spacing: 8
                Rectangle {
                    width: parent.width; height: 36; radius: 3
                    color: expB1.containsMouse ? Qt.darker(AppTheme.ink, 1.15) : AppTheme.ink
                    Rectangle {
                        anchors { fill: parent; topMargin: 3 }
                        radius: 3
                        color: Qt.rgba(28/255, 25/255, 18/255, expB1.containsMouse ? 0.25 : 0.12)
                        z: -1
                        Behavior on color { ColorAnimation { duration: 150 } }
                    }
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("↓  导出当前页")
                        color: "#f4f1ea"
                        font.family: AppTheme.fontFamily; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1
                    }
                    MouseArea {
                        id: expB1
                        anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: exportSettingsDialog.open()
                    }
                }
                Rectangle {
                    width: parent.width; height: 34; radius: 3
                    color: expB2.containsMouse ? AppTheme.signalSoft : "transparent"
                    border.width: 1; border.color: expB2.containsMouse ? AppTheme.signal : AppTheme.line
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("⇊  导出全部页")
                        color: expB2.containsMouse ? AppTheme.signalDeep : AppTheme.ink2
                        font.family: AppTheme.fontFamily; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1
                    }
                    MouseArea {
                        id: expB2
                        anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: exportAllDialog.open()
                    }
                }
            }
        }
    }

    // ==================== 胶片栏 ====================
    Item {
        id: filmstripWrap
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; leftMargin: 18; rightMargin: 18; bottomMargin: 14 }
        height: 80
        opacity: 0

        AcrylicPanel {
            anchors.fill: parent
            showScrews: false

            Row {
                anchors { left: parent.left; leftMargin: 18; verticalCenter: parent.verticalCenter }
                spacing: 14
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    Text { text: "FILMSTRIP"; color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 9; font.bold: true; font.letterSpacing: 2.2 }
                    Text { text: qsTr("页队列"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 12; font.bold: true }
                }
                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 10
                    Repeater {
                        model: app.pageCount
                        delegate: Item {
                            required property int index
                            width: 78; height: 52
                            // 悬停浮起
                            y: pgHover.containsMouse ? -3 : 0
                            Behavior on y { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                            scale: pgHover.containsMouse ? 1.06 : 1.0
                            Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
                            z: pgHover.containsMouse ? 2 : (app.currentPageIndex === index ? 1 : 0)

                            Rectangle {
                                anchors.fill: parent
                                color: "#ffffff"
                                border.width: app.currentPageIndex === index ? 0 : 1
                                border.color: AppTheme.line
                            }
                            Rectangle {
                                anchors.fill: parent; anchors.margins: -3
                                color: "transparent"
                                border.width: 2; border.color: AppTheme.signal
                                visible: app.currentPageIndex === index
                            }
                            Image {
                                anchors.fill: parent; anchors.margins: 2
                                source: app.pageThumbnailUrl(index)
                                sourceSize.width: 156; sourceSize.height: 104
                                cache: false; fillMode: Image.PreserveAspectFit
                            }
                            Rectangle {
                                anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                                height: 12
                                color: Qt.rgba(28/255, 25/255, 18/255, 0.78)
                                Text {
                                    anchors.centerIn: parent
                                    text: "PG-" + (index + 1).toString().padStart(3, "0")
                                    color: "#efece4"
                                    font.family: AppTheme.fontFamily; font.pixelSize: 8; font.letterSpacing: 1.5
                                }
                            }
                            MouseArea {
                                id: pgHover
                                anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                onClicked: app.setCurrentPage(index)
                            }
                        }
                    }
                    // 新建页
                    Rectangle {
                        width: 78; height: 52
                        y: addH.containsMouse ? -2 : 0
                        Behavior on y { NumberAnimation { duration: 150 } }
                        color: addH.containsMouse ? AppTheme.signalSoft : "transparent"
                        border.width: 1
                        border.color: addH.containsMouse ? AppTheme.signal : AppTheme.line
                        Text {
                            anchors.centerIn: parent; text: "＋"
                            color: addH.containsMouse ? AppTheme.signalDeep : AppTheme.ink3
                            font.pixelSize: 16
                        }
                        MouseArea {
                            id: addH
                            anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: app.addPage()
                        }
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("连续排多页 · 整队导出")
                    color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 10; font.letterSpacing: 1.2
                }
            }
        }
    }

    // ==================== 状态吐司 ====================
    AcrylicPanel {
        id: toast
        visible: app.statusMessage.length > 0
        anchors { right: rightPanelWrap.left; rightMargin: 16; bottom: filmstripWrap.top; bottomMargin: 14 }
        height: 38
        width: toastRow.width + 36
        showScrews: false
        opacity: visible ? 1 : 0
        scale: visible ? 1.0 : 0.92
        Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
        Rectangle {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            width: 3; color: AppTheme.signal
        }
        Row {
            id: toastRow
            anchors.centerIn: parent
            spacing: 8
            Rectangle {
                width: 13; height: 13; radius: 7; color: AppTheme.signal
                anchors.verticalCenter: parent.verticalCenter
                Text { anchors.centerIn: parent; text: "✓"; color: "#fff"; font.pixelSize: 8; font.bold: true }
            }
            Text {
                text: app.statusMessage; color: AppTheme.ink2
                font.family: AppTheme.fontFamily; font.pixelSize: 11; font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        Timer { interval: 4200; running: toast.visible; onTriggered: app.clearStatus() }
    }

    // ==================== 导出设置弹窗 ====================
    Dialog {
        id: exportSettingsDialog
        anchors.centerIn: Overlay.overlay
        width: 400
        padding: 0
        modal: true
        closePolicy: Popup.CloseOnEscape
        // 入场动效
        opacity: opened ? 1 : 0
        scale: opened ? 1.0 : 0.92
        Behavior on opacity { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }
        Behavior on scale { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }

        Overlay.modal: Rectangle {
            color: Qt.rgba(237/255, 234/255, 228/255, 0.55)
        }

        background: AcrylicPanel {
            cornerRadius: 2; showScrews: true
        }

        contentItem: Column {
            spacing: 0; padding: 22
            Row {
                spacing: 10
                Text {
                    text: qsTr("导出当前页"); color: AppTheme.ink
                    font.family: AppTheme.fontFamily; font.pixelSize: 16; font.bold: true; font.letterSpacing: 2
                }
                Text {
                    text: "EXPORT · " + app.pageLabel; color: AppTheme.ink3
                    font.family: AppTheme.fontFamily; font.pixelSize: 10; font.bold: true; font.letterSpacing: 2.4
                    anchors.baseline: parent.children[0].baseline
                }
            }
            Text {
                topPadding: 10
                text: qsTr("画布 %1 × %2 MM · %3 PPI · 约 %4 × %5 PX")
                        .arg(app.pageWidthMm).arg(app.pageHeightMm)
                        .arg(rightPanel.expPpi)
                        .arg(app.pagePixelWidth(rightPanel.expPpi))
                        .arg(app.pagePixelHeight(rightPanel.expPpi))
                color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 12
            }
            Text {
                topPadding: 12
                text: qsTr("参数在右栏 OUTPUT 调整。")
                color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 11
            }
            Row {
                topPadding: 16; spacing: 12
                Item { width: parent.parent.width - 44 - 180; height: 1 }
                AcrylicToolButton {
                    text: qsTr("取消")
                    onClicked: exportSettingsDialog.close()
                }
                AcrylicToolButton {
                    text: qsTr("导出"); glyph: "↓"; primary: true
                    onClicked: {
                        exportSettingsDialog.close()
                        exportPageDialog.open()
                    }
                }
            }
        }
    }

    // 点击外部清空选择
    MouseArea {
        anchors.fill: parent; z: -1
        onClicked: app.clearSelection()
    }
}
