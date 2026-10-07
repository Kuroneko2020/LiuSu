#pragma once

#include "domain/ProjectDocument.h"

#include <QObject>
#include <QString>

// 预览控制器：把第一页真实渲染成 PNG，供 QML 直接显示。
// 这是渲染管线的可视化验证片，正式界面在子方案 05 替换。
class PreviewController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString previewUrl READ previewUrl CONSTANT)

public:
    explicit PreviewController(QObject* parent = nullptr);

    QString previewUrl() const { return m_previewUrl; }

private:
    static liusu::domain::ProjectDocument buildDemoProject();
    QString writePreviewPng(const QImage& image);

    QString m_previewUrl;
};
