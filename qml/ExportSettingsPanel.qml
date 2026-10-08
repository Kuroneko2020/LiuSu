import QtQuick
import LiuSu

// 控件由能力定义生成；新增设置在领域/服务层实现并注册，不在界面复制参数范围。
Column {
    id: panel
    property var definitions: app.exportOptionDefinitions
    property var values: app.exportOptionValues
    property bool expanded: false
    spacing: 12
    Repeater {
        model: panel.definitions
        delegate: ExportOptionControl {
            required property var modelData
            width: panel.width
            definition: modelData
            currentValue: panel.values[modelData.id]
            visible: (!modelData.advanced || panel.expanded) && (!modelData.visibleWhen || panel.values[modelData.visibleWhen.id] === modelData.visibleWhen.value)
            onValueEdited: (value) => app.setExportOption(modelData.id,value)
        }
    }
    StudioButton { text: panel.expanded ? qsTr("收起详细设置") : qsTr("更多导出设置"); quiet: true; height: 28; onClicked: panel.expanded=!panel.expanded }
}
