import QtQuick
import LiuSu
import QtQuick.Dialogs

// 主页（J 稿）：选布局 → 进入编辑。
// 四个布局展示板（亚克力档案盒），点击选中后浮出手动排版 / 自动填充。
Item {
    id: home

    signal requestEditor()

    // 自动填充选照片
    FileDialog {
        id: autoFillDialog
        title: qsTr("选择要自动填充的照片")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("图片文件 (*.jpg *.jpeg *.png *.webp *.bmp)")]
        property string presetId: ""
        onAccepted: {
            app.startAutoFill(presetId, selectedFiles)
            home.requestEditor()
        }
    }

    // 环境底：暖灰白渐变 + 光斑
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f0ece6" }
            GradientStop { position: 0.55; color: "#e6e2d9" }
            GradientStop { position: 1.0; color: "#dcd7cb" }
        }
    }
    // 环境柔光（J 稿 radial-gradient 光斑）：中心亮、边缘透明，
    // 不得用实心圆角矩形（会产生可见硬边）。
    Canvas {
        id: glowLeft
        x: -80
        y: -120
        width: 640
        height: 420
        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();
            const grad = ctx.createRadialGradient(width / 2, height / 2, 0,
                                                  width / 2, height / 2, width / 2);
            grad.addColorStop(0.0, "rgba(255,255,255,0.55)");
            grad.addColorStop(0.6, "rgba(255,255,255,0.20)");
            grad.addColorStop(1.0, "rgba(255,255,255,0)");
            ctx.fillStyle = grad;
            ctx.fillRect(0, 0, width, height);
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }
    Canvas {
        id: glowRight
        x: parent.width - 440
        y: -60
        width: 520
        height: 380
        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();
            const grad = ctx.createRadialGradient(width / 2, height / 2, 0,
                                                  width / 2, height / 2, width / 2);
            grad.addColorStop(0.0, "rgba(217,142,43,0.10)");
            grad.addColorStop(0.6, "rgba(217,142,43,0.045)");
            grad.addColorStop(1.0, "rgba(217,142,43,0)");
            ctx.fillStyle = grad;
            ctx.fillRect(0, 0, width, height);
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    // ---- 顶栏（亚克力板，与 J 稿一致：含四角螺丝与右侧读数）----
    AcrylicPanel {
        id: topbar
        anchors { left: parent.left; right: parent.right; top: parent.top;
                  leftMargin: 18; rightMargin: 18; topMargin: 14 }
        height: 58

        Row {
            anchors { left: parent.left; leftMargin: 22; verticalCenter: parent.verticalCenter }
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
                text: qsTr("LIUSU IMPRINT")
                color: AppTheme.ink3
                font.family: AppTheme.fontFamily
                font.pixelSize: 9
                font.letterSpacing: 2.2
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
            Item { width: 10; height: 1 }
            // 导航（编辑/设置在无任务时不可用）
            Repeater {
                model: [
                    { label: qsTr("01 主页"), enabled: true },
                    { label: qsTr("02 编辑"), enabled: false },
                    { label: qsTr("03 设置"), enabled: false }
                ]
                delegate: Item {
                    required property var modelData
                    width: navLabel.width
                    height: 26
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        id: navLabel
                        anchors.centerIn: parent
                        text: modelData.label
                        color: modelData.enabled ? AppTheme.ink : AppTheme.ink3
                        opacity: modelData.enabled ? 1 : 0.55
                        font.family: AppTheme.fontFamily
                        font.pixelSize: 13
                        font.bold: true
                    }
                    Rectangle {
                        visible: modelData.enabled
                        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom }
                        width: parent.width
                        height: 2
                        color: AppTheme.signal
                    }
                }
            }
        }

        Text {
            anchors { right: parent.right; rightMargin: 22; verticalCenter: parent.verticalCenter }
            text: qsTr("PAGE 148 × 100 MM") + "   ·   " + qsTr("OUTPUT 300 PPI")
            color: AppTheme.ink3
            font.family: AppTheme.fontFamily
            font.pixelSize: 10
            font.bold: true
            font.letterSpacing: 1.6
        }
    }

    // ---- 品牌区块 ----
    Column {
        id: brandBlock
        // 纵向位置按窗口高度比例，矮窗口下不裁切（自适应屏幕）。
        anchors { left: parent.left; top: parent.top; leftMargin: parent.width / 2 - 420; topMargin: Math.max(72, parent.height * 0.16) }
        Text {
            text: qsTr("留素")
            color: AppTheme.ink
            font.family: AppTheme.fontFamily
            font.pixelSize: 40
            font.bold: true
            font.letterSpacing: 10
        }
        Text {
            text: qsTr("LIUSU LAYOUT SYSTEM")
            color: AppTheme.ink2
            font.family: AppTheme.fontFamily
            font.pixelSize: 12
            font.bold: true
            font.letterSpacing: 4.2
            topPadding: 6
        }
        Text {
            text: qsTr("IMPRINT OS — VER 0.1")
            color: AppTheme.ink3
            font.family: AppTheme.fontFamily
            font.pixelSize: 10
            font.bold: true
            font.letterSpacing: 3.4
            topPadding: 5
        }
    }
    // ---- 布局展示板 ----
    Row {
        id: boardsRow
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top;
                  topMargin: Math.max(232, parent.height * 0.36) }
        spacing: 40
        property int selectedIndex: -1
        // 自适应：窗口窄时整行等比缩小，保证四块展示板始终完整可见。
        readonly property real fitScale: Math.min(1.0, (home.width - 72) / implicitWidth)
        scale: fitScale
        transformOrigin: Item.Top

        Repeater {
            model: app.layoutPresets()
            delegate: LayoutBoard {
                required property var modelData
                required property int index

                width: 232
                height: 312
                code: "LY-0" + (index + 1)
                name: modelData.name
                nameEn: layoutNameEn(modelData.id)
                subtitle: layoutSubtitle(modelData.id, modelData.slotCount)
                presetId: modelData.id
                slotRects: app.presetSlots(modelData.id)
                selected: boardsRow.selectedIndex === index
                // J 稿：四块展示板各自轻微倾斜，像随手摆开的一组装裱件。
                tilt: [-1.2, 0.8, 0, 1.4][index % 4]

                function layoutNameEn(id) {
                    if (id === "single") return "SINGLE"
                    if (id === "two") return "DUO"
                    if (id === "four") return "QUAD"
                    if (id === "nine") return "GRID-9"
                    return ""
                }

                function layoutSubtitle(id, count) {
                    if (id === "single") return "148 × 100 · 1 SLOT"
                    if (id === "two") return "60 × 90 · 2 SLOTS"
                    if (id === "four") return "66 × 44 · 4 SLOTS"
                    if (id === "nine") return "48 × 32 · 9 SLOTS"
                    return count + " SLOTS"
                }

                onActivated: boardsRow.selectedIndex = (boardsRow.selectedIndex === index ? -1 : index)
                onManualRequested: {
                    app.startManual(presetId)
                    home.requestEditor()
                }
                onAutoRequested: {
                    autoFillDialog.presetId = presetId
                    autoFillDialog.open()
                }
            }
        }
    }

}
