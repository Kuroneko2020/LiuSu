import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "."

Rectangle {
    id: slotRoot
    radius: 0
    color: "transparent"
    border.width: selected ? 2 : 1
    clip: false

    property int slotIndex: -1
    property bool hasImage: false
    property bool selected: false
    property string imageSource: ""
    property bool fillCropMode: false
    property int rotationDegrees: 0
    property bool mirrored: false
    property real cropOffsetX: 0
    property real cropOffsetY: 0
    property real dragDeltaX: 0
    property real dragDeltaY: 0
    property bool swapTargetHighlighted: false
    property bool compositionMode: false
    property bool hovered: slotHover.hovered
    readonly property bool canMoveHorizontal: fillCropMode && contentLayer.overflowX > 0
    readonly property bool canMoveVertical: fillCropMode && contentLayer.overflowY > 0
    readonly property bool canAdjustComposition: hasImage && fillCropMode && (canMoveHorizontal || canMoveVertical)

    signal addClicked()
    signal slotClicked()
    signal rotateClicked()
    signal mirrorClicked()
    signal toggleFillMode()
    signal swapRequested(int fromIndex, int toIndex)
    signal contentDragFinished(real dx, real dy)
    signal compositionOffsetSet(real x, real y)
    signal compositionResetRequested()
    signal removePhotoRequested()
    signal swapDragStarted(int fromIndex, point scenePos)
    signal swapDragMoved(point scenePos)
    signal swapDragFinished(point scenePos)

    onSelectedChanged: if (!selected) compositionMode = false
    onFillCropModeChanged: if (!fillCropMode) compositionMode = false
    onHasImageChanged: if (!hasImage) compositionMode = false
    onCanAdjustCompositionChanged: if (!canAdjustComposition) compositionMode = false

    Timer {
        id: offsetCommitTimer
        interval: 80
        repeat: false
        onTriggered: {
            slotRoot.compositionOffsetSet(cropOffsetX + dragDeltaX, cropOffsetY + dragDeltaY)
            slotRoot.dragDeltaX = 0
            slotRoot.dragDeltaY = 0
        }
    }

    border.color: swapTargetHighlighted ? "#ff8c00" : (selected ? "#7f7f7f" : "#c5c5c5")

    HoverHandler { id: slotHover }

    Rectangle {
        id: contentViewport
        anchors.fill: parent
        radius: 0
        color: hasImage ? "#f4f4f4" : (hovered ? "#e8edf3" : "#eef1f4")
        clip: true

        Item {
            id: contentLayer
            readonly property bool quarterTurn: Math.abs(rotationDegrees % 180) === 90
            readonly property real baseSourceW: photo.sourceSize.width > 0 ? photo.sourceSize.width : contentViewport.width
            readonly property real baseSourceH: photo.sourceSize.height > 0 ? photo.sourceSize.height : contentViewport.height
            readonly property real sourceW: quarterTurn ? baseSourceH : baseSourceW
            readonly property real sourceH: quarterTurn ? baseSourceW : baseSourceH
            readonly property real imageAspect: sourceH > 0 ? sourceW / sourceH : 1
            readonly property real viewportAspect: contentViewport.height > 0 ? contentViewport.width / contentViewport.height : 1
            readonly property real fittedWidth: imageAspect >= viewportAspect ? contentViewport.width : contentViewport.height * imageAspect
            readonly property real fittedHeight: imageAspect >= viewportAspect ? contentViewport.width / imageAspect : contentViewport.height
            readonly property real croppedWidth: imageAspect >= viewportAspect ? contentViewport.height * imageAspect : contentViewport.width
            readonly property real croppedHeight: imageAspect >= viewportAspect ? contentViewport.height : contentViewport.width / imageAspect
            readonly property real layerWidth: fillCropMode ? croppedWidth : fittedWidth
            readonly property real layerHeight: fillCropMode ? croppedHeight : fittedHeight
            readonly property real overflowX: Math.max(0, layerWidth - contentViewport.width)
            readonly property real overflowY: Math.max(0, layerHeight - contentViewport.height)
            readonly property real liveOffsetX: fillCropMode && overflowX > 0 ? Math.max(-1.0, Math.min(1.0, cropOffsetX + dragDeltaX)) : 0
            readonly property real liveOffsetY: fillCropMode && overflowY > 0 ? Math.max(-1.0, Math.min(1.0, cropOffsetY + dragDeltaY)) : 0

            width: Math.max(1, layerWidth)
            height: Math.max(1, layerHeight)
            x: (contentViewport.width - width) * 0.5 + (fillCropMode ? liveOffsetX * overflowX * 0.5 : 0)
            y: (contentViewport.height - height) * 0.5 + (fillCropMode ? liveOffsetY * overflowY * 0.5 : 0)

            Image {
                id: photo
                anchors.fill: parent
                source: imageSource
                visible: hasImage && source !== ""
                fillMode: Image.Stretch
                smooth: true
                cache: true
                transform: [
                    Translate { x: photo.width / 2; y: photo.height / 2 },
                    Rotation { angle: rotationDegrees; origin.x: 0; origin.y: 0 },
                    Scale { xScale: mirrored ? -1 : 1; yScale: 1; origin.x: 0; origin.y: 0 },
                    Translate { x: -photo.width / 2; y: -photo.height / 2 }
                ]
            }

            MouseArea {
                anchors.fill: contentViewport
                enabled: hasImage && selected && fillCropMode && compositionMode
                property real lastX
                property real lastY
                property real totalDx
                property real totalDy
                onPressed: {
                    lastX = mouse.x
                    lastY = mouse.y
                    totalDx = 0
                    totalDy = 0
                    slotRoot.dragDeltaX = 0
                    slotRoot.dragDeltaY = 0
                }
                onPositionChanged: {
                    const dx = (mouse.x - lastX) / Math.max(1, contentViewport.width)
                    const dy = (mouse.y - lastY) / Math.max(1, contentViewport.height)
                    lastX = mouse.x
                    lastY = mouse.y
                    totalDx = contentLayer.overflowX > 0
                        ? Math.max(-1.0 - cropOffsetX, Math.min(1.0 - cropOffsetX, totalDx + dx * 2.0))
                        : 0
                    totalDy = contentLayer.overflowY > 0
                        ? Math.max(-1.0 - cropOffsetY, Math.min(1.0 - cropOffsetY, totalDy + dy * 2.0))
                        : 0
                    slotRoot.dragDeltaX = totalDx
                    slotRoot.dragDeltaY = totalDy
                }
                onReleased: {
                    slotRoot.contentDragFinished(slotRoot.dragDeltaX, slotRoot.dragDeltaY)
                    slotRoot.dragDeltaX = 0
                    slotRoot.dragDeltaY = 0
                }
                onCanceled: {
                    slotRoot.dragDeltaX = 0
                    slotRoot.dragDeltaY = 0
                }
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: {
            if (hasImage) {
                slotRoot.slotClicked()
            } else {
                slotRoot.addClicked()
            }
        }
    }

    Image {
        anchors.centerIn: parent
        visible: !hasImage
        source: "qrc:/qt/qml/Liusu/res/icons/slot-plus.svg"
        sourceSize.width: 34
        sourceSize.height: 34
        width: 34
        height: 34
        opacity: 0.92
    }

    Rectangle {
        anchors.fill: parent
        visible: hasImage && photo.status === Image.Error
        color: "#2a2a2a"
        Label {
            anchors.centerIn: parent
            text: "图片加载失败"
            color: "#ffb3b3"
        }
    }

    Rectangle {
        id: swapHandle
        visible: hasImage
        width: 22
        height: 22
        radius: 7
        color: "#eef1f4"
        border.color: "#c3cad2"
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 6
        Text { anchors.centerIn: parent; text: compositionMode ? "×" : "⇅"; color: "#374151"; font.pixelSize: 10 }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.OpenHandCursor
            drag.target: null
            onPressed: (mouse) => {
                if (compositionMode) {
                    compositionMode = false
                    return
                }
                cursorShape = Qt.ClosedHandCursor
                const p = swapHandle.mapToItem(null, mouse.x, mouse.y)
                slotRoot.swapDragStarted(slotRoot.slotIndex, Qt.point(p.x, p.y))
            }
            onPositionChanged: (mouse) => {
                if (compositionMode) return
                const p = swapHandle.mapToItem(null, mouse.x, mouse.y)
                slotRoot.swapDragMoved(Qt.point(p.x, p.y))
            }
            onReleased: (mouse) => {
                if (compositionMode) return
                cursorShape = Qt.OpenHandCursor
                const p = swapHandle.mapToItem(null, mouse.x, mouse.y)
                slotRoot.swapDragFinished(Qt.point(p.x, p.y))
            }
        }
    }

    RowLayout {
        visible: selected && hasImage && !compositionMode
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4
        spacing: 3

        IconToolButton {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            icon.source: "qrc:/qt/qml/Liusu/res/icons/rotate-right.svg"
            onClicked: slotRoot.rotateClicked()
            tooltipText: "右转 90°"
        }
        IconToolButton {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            icon.source: "qrc:/qt/qml/Liusu/res/icons/mirror.svg"
            onClicked: slotRoot.mirrorClicked()
            tooltipText: "水平镜像"
        }
        IconToolButton {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            icon.source: fillCropMode
                ? "qrc:/qt/qml/Liusu/res/icons/fit-inside.svg"
                : "qrc:/qt/qml/Liusu/res/icons/fill-crop.svg"
            onClicked: slotRoot.toggleFillMode()
            tooltipText: fillCropMode ? "当前：铺满裁切；点击切换到完整放入" : "当前：完整放入；点击切换到铺满裁切"
        }
        IconToolButton {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            visible: canAdjustComposition
            icon.source: "qrc:/qt/qml/Liusu/res/icons/adjust.svg"
            onClicked: compositionMode = true
            tooltipText: "调整构图"
        }
        Item { Layout.fillWidth: true }
        IconToolButton {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            icon.source: "qrc:/qt/qml/Liusu/res/icons/delete.svg"
            onClicked: slotRoot.removePhotoRequested()
            tooltipText: "删除照片"
        }
    }

    Rectangle {
        visible: selected && hasImage && compositionMode
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4
        color: "transparent"
        border.width: 0
        height: compositionControls.implicitHeight + 2

        ColumnLayout {
            id: compositionControls
            anchors.fill: parent
            spacing: 1

            RowLayout {
                visible: canMoveHorizontal
                Layout.fillWidth: true
                Label { text: "左右"; Layout.preferredWidth: 30; font.pixelSize: 11 }
                Slider {
                    Layout.fillWidth: true
                    from: -1.0
                    to: 1.0
                    value: cropOffsetX + dragDeltaX
                    onMoved: {
                        slotRoot.dragDeltaX = value - cropOffsetX
                        offsetCommitTimer.restart()
                    }
                    background: Rectangle {
                        x: parent.leftPadding
                        y: parent.topPadding + parent.availableHeight / 2 - height / 2
                        width: parent.availableWidth
                        height: 2
                        radius: 1
                        color: "#8a8a8a"
                    }
                    handle: Rectangle {
                        x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                        y: parent.topPadding + parent.availableHeight / 2 - height / 2
                        implicitWidth: 10
                        implicitHeight: 10
                        radius: 5
                        color: "#f5f5f5"
                        border.color: "#5a5a5a"
                    }
                }
            }

            RowLayout {
                visible: canMoveVertical
                Layout.fillWidth: true
                Label { text: "上下"; Layout.preferredWidth: 30; font.pixelSize: 11 }
                Slider {
                    Layout.fillWidth: true
                    from: -1.0
                    to: 1.0
                    value: cropOffsetY + dragDeltaY
                    onMoved: {
                        slotRoot.dragDeltaY = value - cropOffsetY
                        offsetCommitTimer.restart()
                    }
                    background: Rectangle {
                        x: parent.leftPadding
                        y: parent.topPadding + parent.availableHeight / 2 - height / 2
                        width: parent.availableWidth
                        height: 2
                        radius: 1
                        color: "#8a8a8a"
                    }
                    handle: Rectangle {
                        x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                        y: parent.topPadding + parent.availableHeight / 2 - height / 2
                        implicitWidth: 10
                        implicitHeight: 10
                        radius: 5
                        color: "#f5f5f5"
                        border.color: "#5a5a5a"
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: "重置构图"
                    onClicked: {
                        slotRoot.dragDeltaX = 0
                        slotRoot.dragDeltaY = 0
                        offsetCommitTimer.stop()
                        slotRoot.compositionResetRequested()
                    }
                }
            }
        }
    }

    focus: compositionMode
    Keys.onEscapePressed: {
        if (compositionMode) {
            compositionMode = false
            event.accepted = true
        }
    }

}
