import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Rectangle {
    id: canvas
    color: "white"
    border.color: "#cfcfcf"
    radius: 6

    readonly property int slotCount: appController.project.currentTemplateSlotCount
    readonly property int slotsRevision: appController.project.slotsRevision

    Item {
        anchors.fill: parent
        anchors.margins: 12

        Repeater {
            model: canvas.slotCount
            delegate: SlotItem {
                required property int index

                property int _rev: canvas.slotsRevision
                property rect slotRect: {
                    _rev
                    return appController.project.slotRectNormalized(index)
                }
                x: slotRect.x * parent.width
                y: slotRect.y * parent.height
                width: slotRect.width * parent.width
                height: slotRect.height * parent.height

                slotIndex: index
                hasImage: {
                    _rev
                    return appController.project.slotHasImage(index)
                }
                selected: {
                    _rev
                    return appController.project.slotSelected(index)
                }
                imagePath: {
                    _rev
                    return appController.project.slotImagePath(index)
                }
                fillCropMode: {
                    _rev
                    return appController.project.slotFillCrop(index)
                }
                rotationDegrees: {
                    _rev
                    return appController.project.slotRotation(index)
                }
                mirrored: {
                    _rev
                    return appController.project.slotMirrored(index)
                }
                cropOffsetX: {
                    _rev
                    return appController.project.slotOffsetX(index)
                }
                cropOffsetY: {
                    _rev
                    return appController.project.slotOffsetY(index)
                }

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
