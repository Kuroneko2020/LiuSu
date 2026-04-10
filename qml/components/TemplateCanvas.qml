import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Rectangle {
    id: canvas
    color: "white"
    border.color: "#cfcfcf"
    radius: 6

    readonly property int slotCount: appController.project.currentTemplateSlotCount()
    readonly property int columns: slotCount <= 2 ? 1 : (slotCount === 4 ? 2 : 3)

    GridLayout {
        anchors.fill: parent
        anchors.margins: 12
        columns: canvas.columns
        rowSpacing: 8
        columnSpacing: 8

        Repeater {
            model: canvas.slotCount
            delegate: SlotItem {
                required property int index
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: (canvas.width - (canvas.columns - 1) * 8) / canvas.columns
                Layout.preferredHeight: (canvas.height - 8) / 2

                slotIndex: index
                hasImage: appController.project.slotHasImage(index)
                selected: appController.project.slotSelected(index)
                imageLabel: appController.project.slotImageLabel(index)
                fillCropMode: appController.project.selectedSlotInFillCrop() && selected

                onAddClicked: appController.importToSlot(slotIndex)
                onSlotClicked: appController.project.selectSlot(slotIndex)
                onRotateClicked: appController.project.rotateSelectedSlot90()
                onMirrorClicked: appController.project.mirrorSelectedSlot()
                onToggleFillMode: appController.project.toggleSelectedSlotFillMode()
            }
        }
    }
}
