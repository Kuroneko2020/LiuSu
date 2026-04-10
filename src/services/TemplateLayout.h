#pragma once

#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QVector>

namespace pte::layout {

QSizeF physicalSizeMm(int templateChoice);
QVector<QRectF> slotRectsNormalized(int templateChoice);
QVector<QRectF> slotRectsPixels(int templateChoice, const QSize &canvasSize);

} // namespace pte::layout
