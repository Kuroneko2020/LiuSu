import QtQuick
import QtQuick.Shapes
import LiuSu

// 主页布局展示板（J 稿 LY-01..04）：照片页衬在亚克力板后。
// 展示板内含：档案编号行 / 页面预览（真实预设几何） / 操作按钮 / 名称行。
// 操作按钮挂在预览下方，鼠标悬停展示板即浮出（无需先点选）；
// 鼠标移入按钮时按钮自身有放大与投影动效。
//
// 渲染要点：
// - 槽位预览用 QtQuick.Shapes 的 ShapePath + 对角 LinearGradient 填充，
//   Shape 自带抗锯齿（此前的"内层放大旋转矩形 + 父级 clip"会产生硬裁剪锯齿）；
// - 每块板携带独立柔影（多层扩散圆角矩形叠加），随板抬起时留在地面并变淡。
Item {
    id: board

    property string code: ""
    property string name: ""
    property string nameEn: ""          // 英文副名（SINGLE / DUO / QUAD / GRID-9）
    property string subtitle: ""
    property string presetId: ""
    property var slotRects: []           // [{x,y,w,h}] 归一化，来自 AppController
    property bool selected: false
    property real tilt: 0                // 静态倾斜角（J 稿 --tilt），悬停/选中回正

    signal activated()
    signal manualRequested()
    signal autoRequested()

    implicitWidth: 232
    implicitHeight: 312

    // 悬停状态汇总：悬停板体或任一按钮都算"正在交互"，避免鼠标移到按钮上时
    // 板体 hover 丢失导致按钮闪没（hoverArea 与按钮 MouseArea 是并列热区）。
    readonly property bool showActions: hoverArea.containsMouse
                                        || manualArea.containsMouse
                                        || autoArea.containsMouse
    readonly property bool lifted: selected || showActions
    // 板抬起量；柔影据其反向补偿，让影子"留在地面"并随抬起变淡。
    readonly property real lift: lifted ? 6 : 0

    // J 稿演示色（h1..h9 对角渐变三停靠点）：仅用于布局预览示意，非真实照片。
    function demoStop(index, stop) {
        const palette = [
            ["#e8c896", "#cf9455", "#a96a3d"],
            ["#b7c9b0", "#7f9a7d", "#546b56"],
            ["#aeb6c2", "#79839a", "#4d5670"],
            ["#d6b5a4", "#b07d63", "#7c4f3b"],
            ["#d9cfa8", "#b3a267", "#7d7040"],
            ["#c2cfc4", "#8ba391", "#59725f"],
            ["#c5cdd8", "#95a0b4", "#95a0b4"],
            ["#d8c2b6", "#ab8874", "#ab8874"],
            ["#cfc9b4", "#a29c7e", "#a29c7e"]
        ]
        return palette[index % 9][stop]
    }

    // 悬浮感：选中或悬停时整板抬升
    y: -lift
    Behavior on y {
        NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
    }
    // J 稿：展示板带轻微倾斜的"摆件感"，悬停或选中时回正。
    rotation: lifted ? 0 : tilt
    Behavior on rotation {
        NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
    }

    // ---- 独立柔影（每板一份，替代原先整排共用的一条投影）----
    // 多层圆角矩形向外扩散、逐层变淡，近似 J 稿的 box-shadow: 0 16px 38px rgba(28,25,18,.15)。
    // 用 y 补偿板抬升量，使影子留在"地面"；抬起时整体变淡。
    Item {
        id: shadowLayer
        anchors.fill: parent
        anchors.margins: -16
        z: -10
        opacity: board.lifted ? 0.72 : 1.0
        Behavior on opacity {
            NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
        }
        // y 补偿：板抬高 6，影相对下移 6，屏幕上保持原位
        y: board.lift

        Repeater {
            // { 垂直偏移, 外扩, 透明度 } 由密到疏叠加成柔和过渡
            model: [
                { yo: 5,  ex: 1,  a: 0.055 },
                { yo: 8,  ex: 4,  a: 0.045 },
                { yo: 11, ex: 8,  a: 0.035 },
                { yo: 14, ex: 13, a: 0.022 }
            ]
            delegate: Rectangle {
                required property var modelData
                x: -modelData.ex + 16
                y: modelData.yo + 16
                width: board.width + modelData.ex * 2
                height: board.height + modelData.ex * 2
                radius: 4 + modelData.ex
                color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, modelData.a)
            }
        }
    }

    AcrylicPanel {
        anchors.fill: parent
        hoverLift: false
        // 选中：琥珀信号描边
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: 3
            color: "transparent"
            border.width: 2
            border.color: AppTheme.signal
            visible: board.selected
        }
    }

    // 档案编号行
    Text {
        id: codeText
        anchors { left: parent.left; top: parent.top; leftMargin: 16; topMargin: 14 }
        text: board.code
        color: board.selected ? AppTheme.signalDeep : AppTheme.ink
        font.family: AppTheme.fontFamily
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 1.8
    }
    Text {
        anchors { left: codeText.right; leftMargin: 8; baseline: codeText.baseline }
        text: board.name + (board.nameEn.length > 0 ? " · " + board.nameEn : "")
        color: AppTheme.ink2
        font.family: AppTheme.fontFamily
        font.pixelSize: 11
        font.letterSpacing: 1.2
    }

    // 页面预览：按真实预设几何绘制槽位；照片衬在玻璃后（奶雾 + 反光）
    Item {
        id: pagePreview
        anchors.horizontalCenter: parent.horizontalCenter
        y: 50
        width: parent.width * 0.76
        height: width / 1.48

        // 相纸落影（预览下方偏移块，模拟纸张浮起）
        Rectangle {
            anchors { fill: parent; topMargin: 5; leftMargin: 2; rightMargin: -2; bottomMargin: -3 }
            z: -1
            color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.13)
        }
        Rectangle {
            anchors.fill: parent
            color: "#ffffff"
            border.width: 1
            border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.10)
        }
        // 槽位：ShapePath + 对角线性渐变（抗锯齿，无硬裁剪）
        Repeater {
            model: board.slotRects
            delegate: Shape {
                id: slotShape
                required property var modelData
                required property int index
                x: modelData.x * pagePreview.width
                y: modelData.y * pagePreview.height
                width: modelData.width * pagePreview.width
                height: modelData.height * pagePreview.height
                antialiasing: true
                preferredRendererType: Shape.CurveRenderer

                ShapePath {
                    strokeWidth: 0
                    fillGradient: LinearGradient {
                        x1: 0
                        y1: 0
                        x2: slotShape.width
                        y2: slotShape.height
                        GradientStop { position: 0.0; color: board.demoStop(slotShape.index, 0) }
                        GradientStop { position: 0.58; color: board.demoStop(slotShape.index, 1) }
                        GradientStop { position: 1.0; color: board.demoStop(slotShape.index, 2) }
                    }
                    startX: 0
                    startY: 0
                    PathLine { x: slotShape.width; y: 0 }
                    PathLine { x: slotShape.width; y: slotShape.height }
                    PathLine { x: 0; y: slotShape.height }
                    PathLine { x: 0; y: 0 }
                }
            }
        }
        // 玻璃后奶雾：照片隔着一层亚克力看的柔和感（J 稿 .haze）
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(250 / 255, 250 / 255, 248 / 255, 0.26) }
                GradientStop { position: 0.5; color: Qt.rgba(250 / 255, 250 / 255, 248 / 255, 0.10) }
                GradientStop { position: 1.0; color: Qt.rgba(250 / 255, 250 / 255, 248 / 255, 0.22) }
            }
        }
        // 玻璃反光扫带（J 稿 .pane 的斜向高光）
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0) }
                GradientStop { position: 0.50; color: Qt.rgba(1, 1, 1, 0) }
                GradientStop { position: 0.63; color: Qt.rgba(1, 1, 1, 0.26) }
                GradientStop { position: 0.76; color: Qt.rgba(1, 1, 1, 0.04) }
                GradientStop { position: 0.90; color: Qt.rgba(1, 1, 1, 0) }
            }
        }
    }

    // 名称行
    Text {
        id: nameRow
        anchors { horizontalCenter: parent.horizontalCenter; bottom: subtitleText.top; bottomMargin: 3 }
        text: board.name
        color: AppTheme.ink
        font.family: AppTheme.fontFamily
        font.pixelSize: 14
        font.bold: true
        font.letterSpacing: 1.2
    }
    Text {
        id: subtitleText
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 16 }
        text: board.subtitle
        color: AppTheme.ink3
        font.family: AppTheme.fontFamily
        font.pixelSize: 10
        font.letterSpacing: 1.6
    }

    // 操作按钮：位于预览与名称行之间的留白区（大致下方空白处），悬停展示板即浮出。
    // 垂直位置取"预览底 → 名称顶"的中点略偏上，视觉上落在留白中央而不是贴边。
    // 鼠标移入按钮时：按钮放大、下投影加重、底色加深（悬停动效）。
    Row {
        id: actionRow
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: pagePreview.bottom
            // 留白区高度 = 名称行顶端 - 预览底端；取其中点作为按钮中心。
            topMargin: Math.max(18, (nameRow.y - pagePreview.y - pagePreview.height) / 2 - height / 2)
        }
        spacing: 10
        // 悬停板时淡入上浮；鼠标从板移到按钮途中不闪断（showActions 覆盖两处热区）。
        opacity: board.showActions ? 1 : 0
        visible: opacity > 0
        transform: Translate {
            y: board.showActions ? 0 : 6
        }
        Behavior on opacity {
            NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
        }

        // 「手动排版」：白底描边按钮
        Item {
            id: manualBtn
            width: 92
            height: 36

            scale: manualArea.containsMouse ? 1.06 : 1.0
            Behavior on scale {
                NumberAnimation { duration: AppTheme.durFast; easing.type: AppTheme.easingType }
            }
            // 悬停时按钮浮起
            y: manualArea.containsMouse ? -1 : 0
            Behavior on y {
                NumberAnimation { duration: AppTheme.durFast; easing.type: AppTheme.easingType }
            }

            // 悬停投影
            Rectangle {
                anchors { fill: parent; topMargin: 4 }
                radius: 3
                color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, manualArea.containsMouse ? 0.22 : 0)
                z: -1
                Behavior on color { ColorAnimation { duration: AppTheme.durFast } }
            }
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: manualArea.containsMouse ? AppTheme.ink : Qt.rgba(1, 1, 1, 0.96)
                border.width: 1
                border.color: AppTheme.ink
                Behavior on color { ColorAnimation { duration: AppTheme.durFast } }
                Text {
                    anchors.centerIn: parent
                    text: qsTr("手动排版")
                    color: manualArea.containsMouse ? "#f4f1ea" : AppTheme.ink
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 11
                    font.bold: true
                    Behavior on color { ColorAnimation { duration: AppTheme.durFast } }
                }
            }
            MouseArea {
                id: manualArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: board.manualRequested()
            }
        }

        // 「自动填充」：近黑实底按钮（无橙色小块，保持极简）
        Item {
            id: autoBtn
            width: 92
            height: 36

            scale: autoArea.containsMouse ? 1.06 : 1.0
            Behavior on scale {
                NumberAnimation { duration: AppTheme.durFast; easing.type: AppTheme.easingType }
            }
            y: autoArea.containsMouse ? -1 : 0
            Behavior on y {
                NumberAnimation { duration: AppTheme.durFast; easing.type: AppTheme.easingType }
            }

            Rectangle {
                anchors { fill: parent; topMargin: 4 }
                radius: 3
                color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, autoArea.containsMouse ? 0.28 : 0)
                z: -1
                Behavior on color { ColorAnimation { duration: AppTheme.durFast } }
            }
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: autoArea.containsMouse ? "#000000" : AppTheme.ink
                Behavior on color { ColorAnimation { duration: AppTheme.durFast } }
                Text {
                    anchors.centerIn: parent
                    text: qsTr("自动填充")
                    color: "#f4f1ea"
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 11
                    font.bold: true
                }
            }
            MouseArea {
                id: autoArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: board.autoRequested()
            }
        }
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        z: -1
        onClicked: board.activated()
    }
}
