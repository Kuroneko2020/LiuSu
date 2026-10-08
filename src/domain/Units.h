#pragma once

#include <QtGlobal>

namespace liusu::domain {

// 归一化矩形：相对页面的 0..1 坐标，与 PPI、页面实际毫米尺寸无关。
// 布局槽位统一用它表达；只在渲染/导出边界才换算成毫米或像素。
// 这样同一份布局档案在 300/600 PPI、甚至未来其它页面尺寸下语义不变。
struct NormalizedRect {
    qreal x{0.0};
    qreal y{0.0};
    qreal width{0.0};
    qreal height{0.0};

    // 合法性：x/y 在页面内、宽高为正、右/下边不越界（允许 1e-9 浮点误差）。
    bool isValid() const;

    bool operator==(const NormalizedRect& other) const;
};

// mm -> px 的唯一换算点：px = mm / 25.4 * ppi，四舍五入到整数像素。
// 仓库里不允许出现第二处 25.4 换算；导出尺寸必须经由这里推导。
qint64 mmToPixels(qreal mm, int ppi);

// px -> mm 的唯一换算点。与 mmToPixels 互为逆运算（含四舍五入误差）。
qreal pixelsToMm(qint64 px, int ppi);

// 由目标像素宽度与已知毫米宽度反推 PPI（预览尺寸推导用）。
// 换算仍走 25.4，调用方不得自行乘除 25.4。
int ppiForPixelWidth(qint64 pixelWidth, qreal mmWidth);

} // namespace liusu::domain
