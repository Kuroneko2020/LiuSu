import QtQuick

// 共享 20×20 线图标，避免 Unicode 字形在不同字体与系统上变宽或变粗。
Canvas {
    id: icon
    property string kind: "add"
    property color color: "#242a26"
    implicitWidth: 20; implicitHeight: 20
    onKindChanged: requestPaint()
    onColorChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
        const c = getContext("2d"); c.reset(); c.scale(width / 20, height / 20)
        c.strokeStyle = color; c.fillStyle = color; c.lineWidth = 1.45; c.lineCap = "round"; c.lineJoin = "round"
        function line(p) { c.beginPath(); c.moveTo(p[0],p[1]); for (let i=2;i<p.length;i+=2) c.lineTo(p[i],p[i+1]); c.stroke() }
        if (kind === "add") { line([10,4,10,16]); line([4,10,16,10]) }
        else if (kind === "arrow") { line([4,10,16,10]); line([11,5,16,10,11,15]) }
        else if (kind === "back") { line([16,10,4,10]); line([9,5,4,10,9,15]) }
        else if (kind === "download") { line([10,3,10,13]); line([6,9,10,13,14,9]); line([4,14,4,17,16,17,16,14]) }
        else if (kind === "import") { line([10,13,10,3]); line([6,7,10,3,14,7]); line([4,12,4,17,16,17,16,12]) }
        else if (kind === "open") { line([3,15,3,5,8,5,10,7,17,7,15,15,3,15]); line([3,10,16,10]) }
        else if (kind === "save") { line([4,3,14,3,17,6,17,17,3,17,3,3,4,3]); line([6,3,6,8,13,8,13,3]); line([6,17,6,12,14,12,14,17]) }
        else if (kind === "rotate") { c.beginPath(); c.arc(10,10,6,0.3,5.2); c.stroke(); line([10,3,14,4,13,8]) }
        else if (kind === "mirror") { line([10,2,10,18]); line([7,5,3,15,7,15,7,5]); line([13,5,17,15,13,15,13,5]) }
        else if (kind === "fit") { line([3,7,3,3,7,3]); line([13,3,17,3,17,7]); line([17,13,17,17,13,17]); line([7,17,3,17,3,13]); c.strokeRect(6,6,8,8) }
        else if (kind === "trash") { line([3,5,17,5]); line([7,5,7,2,13,2,13,5]); line([5,5,6,17,14,17,15,5]); line([8,8,8,14]); line([12,8,12,14]) }
        else if (kind === "close") { line([5,5,15,15]); line([15,5,5,15]) }
        else if (kind === "check") { line([4,10,8,14,16,6]) }
        else if (kind === "mark") { c.strokeRect(3,3,11,11); line([7,17,17,17,17,7]); line([6,11,9,8,12,11]) }
    }
}
