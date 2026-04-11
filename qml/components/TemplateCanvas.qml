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
    readonly property int contentRevision: appController.project.contentRevision
    property int swapSourceSlot: -1
    property int swapTargetSlot: -1

    signal importSlotRequested(int slotIndex)

    function slotIndexAt(scenePos) {
        for (let i = 0; i < slotRepeater.count; ++i) {
            const item = slotRepeater.itemAt(i)
            if (!item) continue
            const topLeft = item.mapToItem(null, 0, 0)
            const bottomRight = item.mapToItem(null, item.width, item.height)
            if (scenePos.x >= topLeft.x && scenePos.x <= bottomRight.x
                    && scenePos.y >= topLeft.y && scenePos.y <= bottomRight.y) {
                return i
            }
        }
        return -1
    }

    Item {
        anchors.fill: parent
        anchors.margins: 12

        Repeater {
            id: slotRepeater
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
                imageSource: {
                    canvas.contentRevision
                    return appController.slotPreviewSource(index, Math.max(32, Math.floor(width)), Math.max(32, Math.floor(height)))
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

                swapTargetHighlighted: canvas.swapSourceSlot >= 0 && canvas.swapTargetSlot === slotIndex && canvas.swapTargetSlot !== canvas.swapSourceSlot

                onAddClicked: canvas.importSlotRequested(slotIndex)
                onSlotClicked: appController.project.selectSlot(slotIndex)
                onRotateClicked: appController.project.rotateSelectedSlot90()
                onMirrorClicked: appController.project.mirrorSelectedSlot()
                onToggleFillMode: appController.project.toggleSelectedSlotFillMode()
                onSwapRequested: (fromIndex, toIndex) => appController.project.swapOrMoveSlots(fromIndex, toIndex)
                onContentDragged: (dx, dy) => appController.project.adjustSelectedSlotOffset(dx, dy)
                onSwapDragStarted: (fromIndex, scenePos) => {
                    canvas.swapSourceSlot = fromIndex
                    canvas.swapTargetSlot = canvas.slotIndexAt(scenePos)
                }
                onSwapDragMoved: (scenePos) => {
                    if (canvas.swapSourceSlot < 0) return
                    canvas.swapTargetSlot = canvas.slotIndexAt(scenePos)
                }
                onSwapDragFinished: (scenePos) => {
                    if (canvas.swapSourceSlot < 0) return
                    const toIndex = canvas.slotIndexAt(scenePos)
                    if (toIndex >= 0 && toIndex !== canvas.swapSourceSlot) {
                        appController.project.swapOrMoveSlots(canvas.swapSourceSlot, toIndex)
                    }
                    canvas.swapSourceSlot = -1
                    canvas.swapTargetSlot = -1
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            z: -1
            onClicked: appController.project.clearSelection()
        }
    }
}
