import QtQuick
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
    Rectangle {
        width: 640; height: 420; x: -80; y: -120; radius: 320
        color: Qt.rgba(1, 1, 1, 0.55)
        opacity: 0.8
    }
    Rectangle {
        width: 520; height: 380; x: parent.width - 440; y: -60; radius: 260
        color: Qt.rgba(217 / 255, 142 / 255, 43 / 255, 0.09)
    }

    // ---- 顶栏 ----
    Row {
        id: topbar
        anchors { left: parent.left; top: parent.top; leftMargin: 26; topMargin: 18 }
        spacing: 14
        Text {
            text: qsTr("留素")
            color: AppTheme.ink
            font.family: AppTheme.fontFamily
            font.pixelSize: 20
            font.bold: true
            font.letterSpacing: 4
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: qsTr("LIUSU IMPRINT")
            color: AppTheme.ink3
            font.family: AppTheme.fontFamily
            font.pixelSize: 10
            font.letterSpacing: 2.6
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }
        Item { width: 12; height: 1 }
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

    // ---- 品牌区块 ----
    Column {
        id: brandBlock
        anchors { left: parent.left; top: parent.top; leftMargin: parent.width / 2 - 420; topMargin: 128 }
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
    Rectangle {
        anchors { left: brandBlock.right; leftMargin: 26; top: brandBlock.top; topMargin: 10 }
        width: 1
        height: 74
        color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.12)
    }
    Text {
        anchors { left: brandBlock.right; leftMargin: 49; top: brandBlock.top; topMargin: 14 }
        text: qsTr("导入照片，选择布局，\n导出后沿裁切线分割。\n为 6 英寸相纸打印机而生。")
        color: AppTheme.ink2
        font.family: AppTheme.fontFamily
        font.pixelSize: 12
        lineHeight: 1.65
    }

    // ---- 布局展示板 ----
    Row {
        id: boardsRow
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 296 }
        spacing: 40
        property int selectedIndex: -1

        Repeater {
            model: app.layoutPresets()
            delegate: LayoutBoard {
                required property var modelData
                required property int index

                width: 232
                height: 312
                code: "LY-0" + (index + 1)
                name: modelData.name
                subtitle: layoutSubtitle(modelData.id, modelData.slotCount)
                presetId: modelData.id
                slotRects: app.presetSlots(modelData.id)
                selected: boardsRow.selectedIndex === index

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

    // 地面投影
    Rectangle {
        id: ground
        anchors { horizontalCenter: parent.horizontalCenter; top: boardsRow.bottom; topMargin: 8 }
        width: boardsRow.width * 0.8
        height: 34
        radius: 17
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.5; color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.16) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    // ---- 底部提示 ----
    Text {
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 26 }
        text: qsTr("点击展示板选择布局 — 手动排版 逐格放置 / 自动填充 批量导入自动分配")
        color: AppTheme.ink3
        font.family: AppTheme.fontFamily
        font.pixelSize: 12
        font.letterSpacing: 1.5
    }
}
