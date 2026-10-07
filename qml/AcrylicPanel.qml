import QtQuick
import LiuSu

// 亚克力展示板：整个应用的唯一材质基元（界面设计基准·第四节）。
// 结构自下而上：挤出板缘（厚度）→ 板体（奶白半透明）→ 内斜面高光 → 斜向反光 → 四角螺丝。
// 内容放进 default 属性；螺丝与反光不拦截鼠标（enabled: false）。
Item {
    id: root

    property real cornerRadius: 2
    property bool showScrews: true
    property bool showSheen: true
    property bool hoverLift: false          // 悬停浮起
    property real hoverLiftAmount: 2
    readonly property bool hovered: hoverArea.containsMouse
    readonly property color bodyTop: AppTheme.acrylicTop
    readonly property color bodyBottom: AppTheme.acrylicBottom

    default property alias contentData: contentHolder.data

    y: hoverLift && hovered ? -hoverLiftAmount : 0
    Behavior on y {
        NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
    }

    // ---- 挤出板缘（厚度）：右下偏移的两层实色 ----
    Rectangle {
        x: AppTheme.edgeOffsetX
        y: AppTheme.edgeOffsetY
        width: parent.width
        height: parent.height
        radius: root.cornerRadius
        color: AppTheme.acrylicEdgeLight
        z: -2
    }
    Rectangle {
        x: AppTheme.edgeOffsetX + 1
        y: AppTheme.edgeOffsetY + 1
        width: parent.width
        height: parent.height
        radius: root.cornerRadius
        color: AppTheme.acrylicEdgeDark
        z: -3
        opacity: 0.9
    }

    // ---- 板体 ----
    Rectangle {
        id: body
        anchors.fill: parent
        radius: root.cornerRadius
        gradient: Gradient {
            GradientStop { position: 0.0; color: root.bodyTop }
            GradientStop { position: 1.0; color: root.bodyBottom }
        }
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.95)

        // 内斜面高光：内侧一圈白边
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1.5
            radius: root.cornerRadius
            color: "transparent"
            border.width: 1.5
            border.color: Qt.rgba(1, 1, 1, 0.42)
        }
        // 左上内反光
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: root.cornerRadius
            color: "transparent"
            border.width: 2
            border.color: Qt.rgba(1, 1, 1, 0.30)
            opacity: 0.7
        }
    }

    // ---- 斜向反光扫带：旋转的宽矩形，被板体裁剪 ----
    Item {
        anchors.fill: parent
        clip: true
        visible: root.showSheen
        z: 2
        Rectangle {
            width: parent.width * 2.2
            height: parent.height * 1.6
            x: -parent.width * 0.9
            y: -parent.height * 0.3
            rotation: 28
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.30; color: Qt.rgba(1, 1, 1, 0) }
                GradientStop { position: 0.42; color: Qt.rgba(1, 1, 1, 0.34) }
                GradientStop { position: 0.52; color: Qt.rgba(1, 1, 1, 0.06) }
                GradientStop { position: 0.68; color: Qt.rgba(1, 1, 1, 0) }
            }
            enabled: false
        }
    }

    // ---- 四角螺丝 ----
    Canvas {
        id: screws
        anchors.fill: parent
        z: 3
        visible: root.showScrews
        enabled: false
        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();
            const inset = 8;
            const r = 3.2;
            const corners = [
                [inset, inset],
                [width - inset, inset],
                [inset, height - inset],
                [width - inset, height - inset]
            ];
            for (const [cx, cy] of corners) {
                // 投影
                ctx.beginPath();
                ctx.arc(cx + 0.6, cy + 0.9, r, 0, Math.PI * 2);
                ctx.fillStyle = "rgba(28,25,18,0.35)";
                ctx.fill();
                // 螺丝外环
                ctx.beginPath();
                ctx.arc(cx, cy, r, 0, Math.PI * 2);
                ctx.fillStyle = "#b5b1a7";
                ctx.fill();
                // 内圈亮面
                ctx.beginPath();
                ctx.arc(cx, cy, r - 1.1, 0, Math.PI * 2);
                ctx.fillStyle = "#f2f0ea";
                ctx.fill();
                // 一字槽（微斜，像真实拧过）
                ctx.save();
                ctx.translate(cx, cy);
                ctx.rotate(-0.4);
                ctx.fillStyle = "rgba(28,25,18,0.55)";
                ctx.fillRect(-r + 1.2, -0.5, (r - 1.2) * 2, 1.0);
                ctx.restore();
            }
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
    }

    // ---- 内容层 ----
    Item {
        id: contentHolder
        anchors.fill: parent
        z: 4
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: root.hoverLift
        acceptedButtons: Qt.NoButton
    }
}
