#pragma once

#include <QColor>
#include <QObject>
#include <QEasingCurve>

// 界面设计基准（docs/设计/界面设计基准.md）的应用级单例。
//
// 为什么用 C++ 而不是 QML 单例文件：QML 文件单例（pragma Singleton）在本工程
// 的资源/模块组合下（qt_add_qml_module + qmldir prefer 机制）运行时解析失败，
// 属性全部读到 undefined（表现为界面色块发黑）。C++ 单例经
// qmlRegisterSingletonInstance 注册，确定性与可测性都更强。
//
// 全部为只读常量；QML 侧用法不变：`AppTheme.ink` 等。
class AppTheme : public QObject
{
    Q_OBJECT

    // ---- 色彩（暖灰白 · 近黑 · 唯一暖杏金信号色）----
    Q_PROPERTY(QColor bgHi READ bgHi CONSTANT)
    Q_PROPERTY(QColor bgLo READ bgLo CONSTANT)
    Q_PROPERTY(QColor bench READ bench CONSTANT)
    Q_PROPERTY(QColor ink READ ink CONSTANT)
    Q_PROPERTY(QColor ink2 READ ink2 CONSTANT)
    Q_PROPERTY(QColor ink3 READ ink3 CONSTANT)
    Q_PROPERTY(QColor line READ line CONSTANT)
    Q_PROPERTY(QColor lineSoft READ lineSoft CONSTANT)
    Q_PROPERTY(QColor signal READ signalColor CONSTANT)
    Q_PROPERTY(QColor signalDeep READ signalDeep CONSTANT)
    Q_PROPERTY(QColor signalSoft READ signalSoft CONSTANT)
    Q_PROPERTY(QColor danger READ danger CONSTANT)
    Q_PROPERTY(QColor slotEmpty READ slotEmpty CONSTANT)

    // ---- 亚克力材质 ----
    Q_PROPERTY(QColor acrylicTop READ acrylicTop CONSTANT)
    Q_PROPERTY(QColor acrylicBottom READ acrylicBottom CONSTANT)
    Q_PROPERTY(QColor acrylicEdgeLight READ acrylicEdgeLight CONSTANT)
    Q_PROPERTY(QColor acrylicEdgeDark READ acrylicEdgeDark CONSTANT)
    Q_PROPERTY(QColor screwRing READ screwRing CONSTANT)
    Q_PROPERTY(qreal edgeOffsetX READ edgeOffsetX CONSTANT)
    Q_PROPERTY(qreal edgeOffsetY READ edgeOffsetY CONSTANT)

    // ---- 动效（精密仪器手感，禁止弹跳）----
    Q_PROPERTY(int durFast READ durFast CONSTANT)
    Q_PROPERTY(int durBase READ durBase CONSTANT)
    Q_PROPERTY(int durSlow READ durSlow CONSTANT)
    Q_PROPERTY(int easingType READ easingType CONSTANT)

    // ---- 字体 ----
    Q_PROPERTY(QString fontFamily READ fontFamily CONSTANT)
    Q_PROPERTY(QString fontFallback READ fontFallback CONSTANT)

public:
    explicit AppTheme(QObject* parent = nullptr) : QObject(parent) {}

    QColor bgHi() const { return QColor(0xed, 0xea, 0xe4); }
    QColor bgLo() const { return QColor(0xe0, 0xdc, 0xd2); }
    QColor bench() const { return QColor(0xdc, 0xd7, 0xcb); }
    QColor ink() const { return QColor(0x1c, 0x1a, 0x16); }
    QColor ink2() const { return QColor(0x6f, 0x6a, 0x60); }
    QColor ink3() const { return QColor(0xa3, 0x9c, 0x8e); }
    QColor line() const { return QColor(28, 25, 18, 31); }       // ~0.12 透明度
    QColor lineSoft() const { return QColor(28, 25, 18, 8); }    // ~0.03
    QColor signalColor() const { return QColor(0xd9, 0x8e, 0x2b); }
    QColor signalDeep() const { return QColor(0xb9, 0x75, 0x17); }
    QColor signalSoft() const { return QColor(217, 142, 43, 38); } // ~0.15
    QColor danger() const { return QColor(0xb3, 0x40, 0x2e); }
    QColor slotEmpty() const { return QColor(0xec, 0xef, 0xf4); }

    QColor acrylicTop() const { return QColor(253, 253, 251, 204); }    // 0.80
    QColor acrylicBottom() const { return QColor(244, 243, 238, 148); } // 0.58
    QColor acrylicEdgeLight() const { return QColor(0xe9, 0xe6, 0xde); }
    QColor acrylicEdgeDark() const { return QColor(0xc9, 0xc5, 0xb9); }
    QColor screwRing() const { return QColor(0xb5, 0xb1, 0xa7); }
    qreal edgeOffsetX() const { return 3.0; }
    qreal edgeOffsetY() const { return 4.0; }

    int durFast() const { return 150; }
    int durBase() const { return 220; }
    int durSlow() const { return 320; }
    int easingType() const { return static_cast<int>(QEasingCurve::OutCubic); }

    QString fontFamily() const { return QStringLiteral("MiSans"); }
    QString fontFallback() const { return QStringLiteral("Microsoft YaHei UI"); }
};
