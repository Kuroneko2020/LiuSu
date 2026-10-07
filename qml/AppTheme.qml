pragma Singleton
import QtQuick

// 界面设计基准（docs/设计/界面设计基准.md）的 QML 化：
// 颜色 / 字体 / 材质分级参数集中在这里，其它 QML 不得自造色值。
QtObject {
    id: theme

    // ---- 色彩（暖灰白 · 近黑 · 唯一暖杏金信号色）----
    readonly property color bgHi: "#edeae4"
    readonly property color bgLo: "#e0dcd2"
    readonly property color bench: "#dcd7cb"
    readonly property color ink: "#1c1a16"
    readonly property color ink2: "#6f6a60"
    readonly property color ink3: "#a39c8e"
    readonly property color line: "#1f1c151f"        // rgba(28,25,18,.12)
    readonly property color lineSoft: "#121c1a1608"  // rgba(28,25,18,.03)
    readonly property color signal: "#d98e2b"
    readonly property color signalDeep: "#b97517"
    readonly property color signalSoft: "#26d98e2b"
    readonly property color danger: "#b3402e"
    readonly property color slotEmpty: "#eceff4"

    // ---- 亚克力材质 ----
    readonly property color acrylicTop: Qt.rgba(253 / 255, 253 / 255, 251 / 255, 0.80)
    readonly property color acrylicBottom: Qt.rgba(244 / 255, 243 / 255, 238 / 255, 0.58)
    readonly property color acrylicEdgeLight: "#e9e6de"
    readonly property color acrylicEdgeDark: "#c9c5b9"
    readonly property color screwRing: "#b5b1a7"
    readonly property real edgeOffsetX: 3
    readonly property real edgeOffsetY: 4

    // ---- 动效（精密仪器手感，禁止弹跳）----
    readonly property int durFast: 150
    readonly property int durBase: 220
    readonly property int durSlow: 320
    readonly property int easingType: Easing.OutCubic

    // ---- 字体 ----
    // MiSans 随发版打包（部署步在 06）；未安装时回退系统默认中文字体。
    readonly property string fontFamily: "MiSans"
    readonly property string fontFallback: "Microsoft YaHei UI"
}
