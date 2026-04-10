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

    Item {
        anchors.fill: parent
        anchors.margins: 12

        Repeater {
            model: canvas.slotCount
            delegate: SlotItem {
                required property int index

                property rect slotRect: appController.project.slotRectNormalized(index)
                x: slotRect.x * parent.width
                y: slotRect.y * parent.height
                width: slotRect.width * parent.width
                height: slotRect.height * parent.height

                slotIndex: index
                hasImage: appController.project.slotHasImage(index)
                selected: appController.project.slotSelected(index)
                imagePath: appController.project.slotImagePath(index)
                fillCropMode: appController.project.slotFillCrop(index)
                rotationDegrees: appController.project.slotRotation(index)
                mirrored: appController.project.slotMirrored(index)
                cropOffsetX: appController.project.slotOffsetX(index)
                cropOffsetY: appController.project.slotOffsetY(index)

                onAddClicked: appController.importToSlot(slotIndex)
                onSlotClicked: appController.project.selectSlot(slotIndex)
                onRotateClicked: appController.project.rotateSelectedSlot90()
                onMirrorClicked: appController.project.mirrorSelectedSlot()
                onToggleFillMode: appController.project.toggleSelectedSlotFillMode()
                onSwapRequested: (fromIndex, toIndex) => appController.project.swapOrMoveSlots(fromIndex, toIndex)
                onContentDragged: (dx, dy) => appController.project.adjustSelectedSlotOffset(dx, dy)
            }
        }
    }
}
