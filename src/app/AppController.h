#pragma once

#include "domain/ProjectDocument.h"

#include <QCache>
#include <QImage>
#include <QObject>
#include <QSize>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

// 应用控制器：QML 与领域/渲染层之间的薄路由（UI规则：UI 不承载几何真相）。
// 所有槽位几何、变换顺序、导出像素计算都在 domain / services 层完成；
// 本类只做状态持有、命令转发和 QML 通知。
class AppController final : public QObject
{
    Q_OBJECT

    // 当前页修订号：编辑当前页时递增，驱动当前页预览失效重取。
    // 与逐页修订分离，避免一次编辑触发全页缩略图重渲染（性能底线）。
    Q_PROPERTY(int revision READ revision NOTIFY changed)
    Q_PROPERTY(QString previewUrl READ previewUrl NOTIFY changed)
    Q_PROPERTY(int currentPageIndex READ currentPageIndex NOTIFY currentPageChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY changed)
    Q_PROPERTY(QString pageLabel READ pageLabel NOTIFY currentPageChanged)
    Q_PROPERTY(int selectedSlot READ selectedSlot NOTIFY selectionChanged)
    Q_PROPERTY(QVariantMap selectedSlotRect READ selectedSlotRect NOTIFY selectionChanged)
    Q_PROPERTY(QString backgroundHex READ backgroundHex NOTIFY changed)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int currentSlotCount READ currentSlotCount NOTIFY changed)

public:
    explicit AppController(QObject* parent = nullptr);

    int revision() const { return m_pageRevisions.value(m_currentPageIndex, 0) + m_globalRevision; }
    QString previewUrl() const;
    // 胶片栏缩略图 URL：只含该页修订号，编辑其它页不会令本页失效。
    Q_INVOKABLE QString pageThumbnailUrl(int pageIndex) const;
    int currentPageIndex() const { return m_currentPageIndex; }
    int pageCount() const { return m_document.pages.size(); }
    QString pageLabel() const;
    int selectedSlot() const { return m_selectedSlot; }
    QVariantMap selectedSlotRect() const;
    QString backgroundHex() const { return m_document.background.colorHex; }
    QString statusMessage() const { return m_statusMessage; }
    int currentSlotCount() const;
    // 开发预览开关：LIUSU_DEMO_PRESET=four 时启动即建该预设工程并直接进编辑页。
    // 仅用于开发期截图与目检，不作为产品功能；未设置时为空串。
    Q_PROPERTY(QString demoPreset READ demoPreset CONSTANT)
    QString demoPreset() const;

    // 供 PagePreviewProvider 使用：按请求像素宽度推导 PPI 渲染指定页。
    // requestedSize 无效时用回退 PPI。索引越界返回空图。
    QImage renderPageForProvider(int pageIndex, const QSize& requestedSize) const;

    // ---- 布局预设 ----
    Q_INVOKABLE QVariantList layoutPresets() const;              // [{id,name,slotCount}]
    Q_INVOKABLE QVariantList presetSlots(const QString& presetId) const; // [{x,y,w,h} 归一化]

    // ---- 项目流转 ----
    Q_INVOKABLE void startManual(const QString& presetId);       // 新工程 + 预设，进编辑
    Q_INVOKABLE void startAutoFill(const QString& presetId, const QVariantList& fileUrls);

    // ---- 槽位编辑（作用于当前页；fileUrls 为 QUrl） ----
    Q_INVOKABLE void assignFilesToEmptySlots(const QVariantList& fileUrls);
    Q_INVOKABLE void assignFileToSlot(int slotIndex, const QUrl& fileUrl);
    Q_INVOKABLE void rotateSlot(int slotIndex);
    Q_INVOKABLE void mirrorSlot(int slotIndex);
    Q_INVOKABLE void toggleFillMode(int slotIndex);
    Q_INVOKABLE void clearSlot(int slotIndex);
    Q_INVOKABLE void selectSlotAt(qreal normalizedX, qreal normalizedY);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void clearStatus();

    // ---- 页面与背景 ----
    Q_INVOKABLE void addPage();
    Q_INVOKABLE void deleteCurrentPage();
    Q_INVOKABLE void setCurrentPage(int index);
    Q_INVOKABLE void setBackground(const QString& colorHex);

    // ---- 导出与项目文件 ----
    Q_INVOKABLE bool exportCurrentPage(const QUrl& fileUrl, int ppi, bool jpeg, int quality);
    Q_INVOKABLE bool exportAllPages(const QUrl& directoryUrl, int ppi, bool jpeg, int quality);
    Q_INVOKABLE bool saveProject(const QUrl& fileUrl);
    Q_INVOKABLE bool openProject(const QUrl& fileUrl);

signals:
    void changed();
    void currentPageChanged();
    void selectionChanged();
    void statusMessageChanged();

private:
    liusu::domain::ProjectPage* currentPage();
    const liusu::domain::ProjectPage* currentPage() const;
    // 影响当前页像素的编辑：递增当前页修订并清理该页缓存。
    void bumpCurrentPageRevision();
    // 影响所有页像素的变化（背景等）：递增全局修订并清理全部缓存。
    void bumpGlobalRevision();
    void rebuildPageRevisions();
    void setStatus(const QString& message);
    bool profileWidthHeight(int* outWidthMm, int* outHeightMm) const;
    QImage renderPageAt(int pageIndex, int ppi) const;

    liusu::domain::ProjectDocument m_document;
    int m_currentPageIndex = 0;
    int m_selectedSlot = -1;
    QVector<int> m_pageRevisions;      // 与 pages 对齐的逐页修订号
    int m_globalRevision = 0;          // 背景 / 档案级变化
    mutable QCache<QString, QImage> m_pageCache; // 页面预览缓存（key: page|ppi|rev）
    QString m_statusMessage;
};
