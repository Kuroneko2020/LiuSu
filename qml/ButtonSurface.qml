import QtQuick
import LiuSu

// 按键形态控件的统一交互面。
//
// 全应用的按钮 / 色板 / chip / 页卡都套这一层做悬停与按压反馈：位移与缩放
// 只在 AppTheme 定义一次（hoverLift / pressScale / durBase / durFast / easingType），
// 禁止各处另定时长、位移、缩放或缓动。
//
// 本组件只管"怎么动"，不管"长什么样"与"点了做什么"：调用方自带配色、圆角、
// 内容与 MouseArea，并把 hover / press 状态回填进来。
//
// 悬停只做竖直上浮，不做缩放、不做变色（对照 RhineLabUI 的浮起语汇）；
// 按压才回缩到 pressScale，落手感统一。
Rectangle {
    id: root

    property bool hovered: false
    property bool pressed: false
    property bool interactive: true

    // 悬停上浮；按压时先落回原位再回缩，避免"边浮边缩"的双重反馈。
    //
    // 位移走 transform 而不是绑 y：本组件大量被 Grid / Row / Column 定位，
    // 那些布局会自己算子项的 x/y；直接绑 y 会把算好的位置覆盖成 0，
    // 控件整体跳到容器原点（"过度上浮"的根因）。transform 不参与布局排位。
    transform: Translate {
        y: root.interactive && root.hovered && !root.pressed ? -AppTheme.hoverLift : 0
        Behavior on y {
            NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
        }
    }
    scale: interactive && pressed ? AppTheme.pressScale : 1.0
    Behavior on scale {
        NumberAnimation { duration: AppTheme.durFast; easing.type: AppTheme.easingType }
    }
}
