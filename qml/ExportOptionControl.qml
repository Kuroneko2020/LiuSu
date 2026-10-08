import QtQuick
import QtQuick.Controls
import LiuSu

Column {
    id: control
    property var definition: ({})
    property var currentValue
    signal valueEdited(var value)
    spacing: 7
    Text { text: control.definition.label || ""; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 12 }
    Loader {
        width: parent.width
        sourceComponent: control.definition.type === "choice" ? choices : control.definition.type === "integer" ? integer : unsupported
    }
    Text { width: parent.width; visible: text.length>0; text: control.definition.help || ""; wrapMode: Text.WordWrap; color: AppTheme.ink3; font.family: AppTheme.fontFamily; font.pixelSize: 10 }
    Component {
        id: choices
        Flow {
            spacing: 6
            Repeater {
                model: control.definition.choices || []
                delegate: StudioButton {
                    required property var modelData
                    text: modelData.label
                    primary: modelData.value === control.currentValue
                    onClicked: control.valueEdited(modelData.value)
                }
            }
        }
    }
    Component {
        id: integer
        Column {
            spacing: 6
            Flow {
                width: parent.width; spacing: 6
                Repeater {
                    model: control.definition.presets || []
                    delegate: StudioButton {
                        required property var modelData
                        text: modelData.toString(); primary: modelData === control.currentValue
                        onClicked: control.valueEdited(modelData)
                    }
                }
            }
            SpinBox {
                objectName: "exportOption-"+control.definition.id
                width: parent.width; editable: true
                from: control.definition.min; to: control.definition.max
                value: control.currentValue === undefined ? from : control.currentValue
                onValueModified: control.valueEdited(value)
                Accessible.name: control.definition.label
            }
        }
    }
    Component { id: unsupported; Text { text: qsTr("此设置需要更新应用后使用"); color: AppTheme.ink3; font.pixelSize: 11 } }
}
