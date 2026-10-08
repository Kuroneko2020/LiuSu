import QtQuick
import QtQuick.Controls
import LiuSu

Rectangle {
    id: inspector
    signal replaceRequested()
    signal exportPageRequested()
    signal exportAllRequested()
    signal backgroundRequested()
    property bool advancedExport: false
    color: "#f6f7f1"
    border.width: 1; border.color: "#d7dccf"; radius: 3
    readonly property var selection: app.selectedSlotState
    function showExport() { scroll.contentItem.contentY = Math.max(0,outputSection.y-20) }
    Connections { target: app; function onCurrentPageChanged() { scroll.contentItem.contentY = 0 } }

    ScrollView {
        id: scroll
        objectName: "inspectorScroll"
        anchors.fill: parent
        anchors.margins: 18
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AsNeeded
        Column {
            width: scroll.availableWidth
            spacing: 16
            Column {
                width: parent.width; spacing: 6
                SectionLabel { text: "LAYOUT / 当前页模板"; color: AppTheme.ink }
                Text { text: qsTr("第 %1 页 · %2").arg(app.currentPageIndex+1).arg(app.currentLayoutName); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 18; font.weight: Font.Medium }
                Text { text: qsTr("每页独立设置，随时可以更换"); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
            }
            TemplatePicker { width: parent.width; selectedId: app.currentLayoutId; onPicked: (presetId) => app.changeCurrentLayout(presetId) }
            Rectangle { width: parent.width; height: 1; color: AppTheme.line }
            Column {
                width: parent.width; spacing: 10
                SectionLabel { text: "PHOTOGRAPH / 照片"; color: AppTheme.ink }
                Text {
                    width: parent.width
                    text: app.selectedSlot >= 0 ? (inspector.selection.hasImage ? inspector.selection.name : qsTr("空槽位 %1").arg(app.selectedSlot+1)) : qsTr("选中一张照片，开始调整")
                    elide: Text.ElideMiddle; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 12
                }
                Row {
                    visible: !!inspector.selection.hasImage
                    spacing: 6
                    StudioButton { text: qsTr("旋转"); iconName: "rotate"; enabled: !!inspector.selection.hasImage; onClicked: app.rotateSlot(app.selectedSlot) }
                    StudioButton { text: qsTr("镜像"); iconName: "mirror"; enabled: !!inspector.selection.hasImage; onClicked: app.mirrorSlot(app.selectedSlot) }
                }
                Row {
                    visible: app.selectedSlot >= 0
                    spacing: 6
                    StudioButton { text: qsTr("替换"); iconName: "import"; enabled: app.selectedSlot>=0; onClicked: inspector.replaceRequested() }
                    StudioButton { text: qsTr("移除"); iconName: "close"; quiet: true; enabled: !!inspector.selection.hasImage; onClicked: app.clearSlot(app.selectedSlot) }
                }
                StudioButton {
                    width: parent.width
                    visible: !!inspector.selection.hasImage
                    text: inspector.selection.fill ? qsTr("铺满裁切 · 切换为完整显示") : qsTr("完整显示 · 切换为铺满裁切")
                    iconName: "fit"; enabled: !!inspector.selection.hasImage
                    onClicked: app.toggleFillMode(app.selectedSlot)
                }
                Column {
                    width: parent.width; spacing: 2
                    visible: !!inspector.selection.hasImage && !!inspector.selection.fill
                    Text { text: qsTr("取景位置"); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
                    Slider { width: parent.width; from: -1; to: 1; value: inspector.selection.cropX || 0; onMoved: app.setCropOffset(app.selectedSlot,value,inspector.selection.cropY); Accessible.name: qsTr("水平取景") }
                    Slider { width: parent.width; from: -1; to: 1; value: inspector.selection.cropY || 0; onMoved: app.setCropOffset(app.selectedSlot,inspector.selection.cropX,value); Accessible.name: qsTr("垂直取景") }
                    StudioButton { text: qsTr("取景复位"); quiet: true; height: 28; onClicked: app.setCropOffset(app.selectedSlot,0,0) }
                }
            }
            Rectangle { width: parent.width; height: 1; color: AppTheme.line }
            Column {
                width: parent.width; spacing: 10
                SectionLabel { text: "PAPER / 项目纸面背景"; color: AppTheme.ink }
                Row {
                    spacing: 7
                    Repeater {
                        model: ["#ffffff","#f4efe5","#e6e9e0","#dce5e8","#252a27"]
                        delegate: Button {
                            required property string modelData
                            width: 30; height: 30
                            Accessible.name: qsTr("纸面背景 %1").arg(modelData)
                            background: Rectangle { color: modelData; border.width: app.backgroundHex===modelData ? 2 : 1; border.color: app.backgroundHex===modelData ? AppTheme.signal : "#cbd1c4"; radius: 2 }
                            onClicked: app.setBackground(modelData)
                        }
                    }
                }
                StudioButton { text: qsTr("自定义颜色"); quiet: true; height: 28; onClicked: inspector.backgroundRequested() }
            }
            Rectangle { width: parent.width; height: 1; color: AppTheme.line }
            Column {
                id: outputSection
                objectName: "outputSection"
                width: parent.width; spacing: 10
                SectionLabel { text: "OUTPUT / 导出"; color: AppTheme.ink }
                Row {
                    spacing: 6
                    Repeater {
                        model: [300,600]
                        delegate: StudioButton {
                            required property int modelData
                            text: modelData + " PPI"; primary: app.exportPpi===modelData
                            onClicked: app.setExportSettings(modelData,app.exportJpeg,app.exportQuality)
                        }
                    }
                }
                Row {
                    spacing: 6
                    StudioButton { text: "JPEG"; primary: app.exportJpeg; onClicked: app.setExportSettings(app.exportPpi,true,app.exportQuality) }
                    StudioButton { text: "PNG"; primary: !app.exportJpeg; onClicked: app.setExportSettings(app.exportPpi,false,app.exportQuality) }
                }
                Text { text: app.pagePixelWidth(app.exportPpi) + " × " + app.pagePixelHeight(app.exportPpi) + qsTr(" 像素"); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
                StudioButton { text: inspector.advancedExport ? qsTr("收起详细设置") : qsTr("更多导出设置"); quiet: true; height: 28; onClicked: inspector.advancedExport = !inspector.advancedExport }
                Column {
                    width: parent.width; spacing: 6
                    visible: inspector.advancedExport || (app.exportPpi !== 300 && app.exportPpi !== 600)
                    Text { text: qsTr("自定义 PPI · 72–1200"); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
                    SpinBox { objectName: "customPpiInput"; width: parent.width; from: 72; to: 1200; editable: true; value: app.exportPpi; onValueModified: app.setExportSettings(value,app.exportJpeg,app.exportQuality); Accessible.name: qsTr("自定义 PPI") }
                    Text { visible: app.exportJpeg; text: qsTr("JPEG 质量 · %1").arg(app.exportQuality); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
                    Slider { visible: app.exportJpeg; width: parent.width; from: 1; to: 100; stepSize: 1; value: app.exportQuality; onMoved: app.setExportSettings(app.exportPpi,true,Math.round(value)); Accessible.name: qsTr("JPEG 质量") }
                }
                StudioButton { width: parent.width; text: qsTr("导出当前页"); iconName: "download"; primary: true; onClicked: inspector.exportPageRequested() }
                StudioButton { width: parent.width; text: qsTr("导出全部 %1 页").arg(app.pageCount); onClicked: inspector.exportAllRequested() }
            }
            Item { width: 1; height: 8 }
        }
    }
}
