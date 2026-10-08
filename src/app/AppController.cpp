#include "AppController.h"

#include "domain/Layout.h"
#include "domain/Page.h"
#include "domain/Units.h"
#include "services/render/PageRenderer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QUrl>

using liusu::domain::ProjectPage;
using liusu::domain::ProjectDocument;
using liusu::domain::SlotImageState;
using liusu::domain::FillMode;
namespace LayoutPresets = liusu::domain::LayoutPresets;
namespace Profiles = liusu::domain::Profiles;
namespace PageRenderer = liusu::render::PageRenderer;

namespace {
constexpr int kPreviewFallbackPpi = 96;
constexpr int kPreviewMinPpi = 48;
constexpr int kPreviewMaxPpi = 300;
// 页面预览缓存上限：约容纳十余张整页位图；超出按 LRU 淘汰。
constexpr int kPageCacheCostBytes = 64 * 1024 * 1024;
} // namespace

AppController::AppController(QObject* parent)
    : QObject(parent)
{
    m_document = ProjectDocument::createDefault();
    m_pageCache.setMaxCost(kPageCacheCostBytes);
    rebuildPageRevisions();

    // 开发预览：构建演示工程（四宫格 + 测试图集），供启动直达编辑页的目检。
    const QString presetId = demoPreset();
    if (!presetId.isEmpty()) {
        startManual(presetId);
        ProjectPage* page = currentPage();
        if (page) {
            const QStringList demoImages = {
                QStringLiteral(":/images/exif1.jpg"), QStringLiteral(":/images/exif6.jpg"),
                QStringLiteral(":/images/exif3.jpg"), QStringLiteral(":/images/exif8.jpg"),
            };
            for (int i = 0; i < page->slotStates.size() && i < demoImages.size(); ++i) {
                page->slotStates[i].imagePath = demoImages.at(i);
                page->slotStates[i].fillMode = FillMode::Fill;
            }
            bumpCurrentPageRevision();
        }
    }
}

QString AppController::demoPreset() const
{
    return qEnvironmentVariable("LIUSU_DEMO_PRESET");
}

QString AppController::previewUrl() const
{
    return QStringLiteral("image://liusu/page/%1?rev=%2")
        .arg(m_currentPageIndex)
        .arg(revision());
}

QString AppController::pageThumbnailUrl(int pageIndex) const
{
    // 每页仅携带自己的修订号 + 全局修订（背景变化影响所有页）。
    const int pageRev = m_pageRevisions.value(pageIndex, 0);
    return QStringLiteral("image://liusu/page/%1?rev=%2")
        .arg(pageIndex)
        .arg(pageRev + m_globalRevision);
}

QString AppController::pageLabel() const
{
    return QStringLiteral("PG-%1").arg(m_currentPageIndex + 1, 3, 10, QLatin1Char('0'));
}

QVariantMap AppController::selectedSlotRect() const
{
    QVariantMap map;
    const ProjectPage* page = currentPage();
    if (!page || m_selectedSlot < 0 || m_selectedSlot >= page->layout.slotRects.size())
        return map;
    const liusu::domain::NormalizedRect& rect = page->layout.slotRects.at(m_selectedSlot);
    map.insert(QStringLiteral("x"), rect.x);
    map.insert(QStringLiteral("y"), rect.y);
    map.insert(QStringLiteral("width"), rect.width);
    map.insert(QStringLiteral("height"), rect.height);
    return map;
}

QImage AppController::renderPageForProvider(int pageIndex, const QSize& requestedSize) const
{
    if (pageIndex < 0 || pageIndex >= m_document.pages.size())
        return {};

    int ppi = kPreviewFallbackPpi;
    qreal widthMm = 0.0;
    qreal heightMm = 0.0;
    profileWidthHeight(&widthMm, &heightMm);
    if (requestedSize.isValid() && requestedSize.width() > 0 && widthMm > 0.0) {
        const int derived = liusu::domain::ppiForPixelWidth(requestedSize.width(), widthMm);
        ppi = qBound(kPreviewMinPpi, derived, kPreviewMaxPpi);
    }
    return renderPageAt(pageIndex, ppi);
}

QVariantList AppController::layoutPresets() const
{
    QVariantList result;
    const auto presets = LayoutPresets::builtinPresets();
    for (const auto& info : presets) {
        bool ok = false;
        const auto layout = LayoutPresets::create(info.id, &ok);
        QVariantMap map;
        map.insert(QStringLiteral("id"), info.id);
        map.insert(QStringLiteral("name"), info.displayName);
        map.insert(QStringLiteral("slotCount"), ok ? layout.slotRects.size() : 0);
        result.append(map);
    }
    return result;
}

QVariantList AppController::presetSlots(const QString& presetId) const
{
    QVariantList result;
    bool ok = false;
    const auto layout = LayoutPresets::create(presetId, &ok);
    if (!ok)
        return result;
    for (const auto& rect : layout.slotRects) {
        QVariantMap map;
        map.insert(QStringLiteral("x"), rect.x);
        map.insert(QStringLiteral("y"), rect.y);
        map.insert(QStringLiteral("width"), rect.width);
        map.insert(QStringLiteral("height"), rect.height);
        result.append(map);
    }
    return result;
}

void AppController::startManual(const QString& presetId)
{
    bool ok = false;
    const auto layout = LayoutPresets::create(presetId, &ok);
    if (!ok) {
        setStatus(QStringLiteral("未知布局预设"));
        return;
    }

    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage page;
    page.layout = layout;
    for (int i = 0; i < layout.slotRects.size(); ++i)
        page.slotStates.append(SlotImageState{});
    doc.pages = { page };
    m_document = doc;
    m_currentPageIndex = 0;
    m_selectedSlot = -1;
    emit selectionChanged();
    emit currentPageChanged();
    bumpGlobalRevision();
    setStatus(QStringLiteral("已建立 %1 · 手动排版").arg(presetId));
}

void AppController::startAutoFill(const QString& presetId, const QVariantList& fileUrls)
{
    bool ok = false;
    const auto layout = LayoutPresets::create(presetId, &ok);
    if (!ok) {
        setStatus(QStringLiteral("未知布局预设"));
        return;
    }

    ProjectDocument doc = ProjectDocument::createDefault();
    ProjectPage page;
    page.layout = layout;
    for (int i = 0; i < layout.slotRects.size(); ++i)
        page.slotStates.append(SlotImageState{});

    // 自动填充：按选择顺序循环填入所有槽位（铺满裁切为默认，符合打印裁切预期）。
    int fileIndex = 0;
    for (SlotImageState& state : page.slotStates) {
        if (fileIndex >= fileUrls.size())
            break;
        const QUrl url = fileUrls.at(fileIndex).toUrl();
        state.imagePath = url.toLocalFile();
        state.fillMode = FillMode::Fill;
        ++fileIndex;
    }
    doc.pages = { page };
    m_document = doc;
    m_currentPageIndex = 0;
    m_selectedSlot = -1;
    emit selectionChanged();
    emit currentPageChanged();
    bumpGlobalRevision();
    setStatus(QStringLiteral("已自动填充 %1 张照片").arg(qMin(fileUrls.size(), layout.slotRects.size())));
}

void AppController::assignFilesToEmptySlots(const QVariantList& fileUrls)
{
    ProjectPage* page = currentPage();
    if (!page)
        return;

    int assigned = 0;
    int fileIndex = 0;
    for (SlotImageState& state : page->slotStates) {
        if (fileIndex >= fileUrls.size())
            break;
        if (!state.imagePath.isEmpty())
            continue;
        state.imagePath = fileUrls.at(fileIndex).toUrl().toLocalFile();
        state.fillMode = FillMode::Fill;
        ++fileIndex;
        ++assigned;
    }
    bumpCurrentPageRevision();
    setStatus(assigned > 0 ? QStringLiteral("已填充 %1 个空槽位").arg(assigned)
                           : QStringLiteral("没有可用空槽位"));
}

void AppController::assignFileToSlot(int slotIndex, const QUrl& fileUrl)
{
    ProjectPage* page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size())
        return;
    page->slotStates[slotIndex].imagePath = fileUrl.toLocalFile();
    page->slotStates[slotIndex].fillMode = FillMode::Fill;
    bumpCurrentPageRevision();
    setStatus(QStringLiteral("已替换槽位 %1 的照片").arg(slotIndex + 1));
}

void AppController::rotateSlot(int slotIndex)
{
    ProjectPage* page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size())
        return;
    SlotImageState& state = page->slotStates[slotIndex];
    state.rotationDegrees = (state.rotationDegrees + 90) % 360;
    bumpCurrentPageRevision();
}

void AppController::mirrorSlot(int slotIndex)
{
    ProjectPage* page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size())
        return;
    page->slotStates[slotIndex].mirrored = !page->slotStates[slotIndex].mirrored;
    bumpCurrentPageRevision();
}

void AppController::toggleFillMode(int slotIndex)
{
    ProjectPage* page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size())
        return;
    SlotImageState& state = page->slotStates[slotIndex];
    state.fillMode = state.fillMode == FillMode::Fill ? FillMode::Fit : FillMode::Fill;
    bumpCurrentPageRevision();
}

void AppController::clearSlot(int slotIndex)
{
    ProjectPage* page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size())
        return;
    page->slotStates[slotIndex] = SlotImageState{};
    bumpCurrentPageRevision();
}

void AppController::selectSlotAt(qreal normalizedX, qreal normalizedY)
{
    const ProjectPage* page = currentPage();
    if (!page)
        return;
    int hit = -1;
    for (int i = 0; i < page->layout.slotRects.size(); ++i) {
        const auto& rect = page->layout.slotRects.at(i);
        if (normalizedX >= rect.x && normalizedX <= rect.x + rect.width
            && normalizedY >= rect.y && normalizedY <= rect.y + rect.height) {
            hit = i;
            break;
        }
    }
    if (hit != m_selectedSlot) {
        m_selectedSlot = hit;
        emit selectionChanged();
    }
}

void AppController::clearSelection()
{
    if (m_selectedSlot != -1) {
        m_selectedSlot = -1;
        emit selectionChanged();
    }
}

void AppController::clearStatus()
{
    setStatus(QString());
}

int AppController::currentSlotCount() const
{
    const ProjectPage* page = currentPage();
    return page ? page->layout.slotRects.size() : 0;
}

qreal AppController::pageWidthMm() const
{
    qreal w = 148.0;
    profileWidthHeight(&w, nullptr);
    return w;
}

qreal AppController::pageHeightMm() const
{
    qreal h = 100.0;
    profileWidthHeight(nullptr, &h);
    return h;
}

int AppController::pagePixelWidth(int ppi) const
{
    qreal w = 148.0;
    profileWidthHeight(&w, nullptr);
    return static_cast<int>(liusu::domain::mmToPixels(w, ppi));
}

int AppController::pagePixelHeight(int ppi) const
{
    qreal h = 100.0;
    profileWidthHeight(nullptr, &h);
    return static_cast<int>(liusu::domain::mmToPixels(h, ppi));
}

void AppController::addPage()
{
    ProjectPage page;
    // 新页沿用当前页布局；照片从空开始，避免下意识复制造成误导出。
    if (const ProjectPage* current = currentPage())
        page.layout = current->layout;
    else
        page.layout = LayoutPresets::create(QStringLiteral("single"));
    for (int i = 0; i < page.layout.slotRects.size(); ++i)
        page.slotStates.append(SlotImageState{});
    m_document.pages.append(page);
    m_currentPageIndex = m_document.pages.size() - 1;
    m_selectedSlot = -1;
    emit selectionChanged();
    emit currentPageChanged();
    rebuildPageRevisions();
    emit changed();
    setStatus(QStringLiteral("已新建 %1").arg(pageLabel()));
}

void AppController::deleteCurrentPage()
{
    if (m_document.pages.size() <= 1) {
        setStatus(QStringLiteral("至少保留一页"));
        return;
    }
    m_document.pages.removeAt(m_currentPageIndex);
    m_currentPageIndex = qBound(0, m_currentPageIndex, m_document.pages.size() - 1);
    m_selectedSlot = -1;
    emit selectionChanged();
    emit currentPageChanged();
    m_pageCache.clear();
    emit changed();
    setStatus(QStringLiteral("已删除页"));
}

void AppController::setCurrentPage(int index)
{
    if (index < 0 || index >= m_document.pages.size() || index == m_currentPageIndex)
        return;
    m_currentPageIndex = index;
    m_selectedSlot = -1;
    emit selectionChanged();
    emit currentPageChanged();
    emit changed();   // 预览 URL 指向当前页，导航即失效；不动修订号与缓存
}

void AppController::setBackground(const QString& colorHex)
{
    if (m_document.background.colorHex == colorHex)
        return;
    m_document.background.colorHex = colorHex;
    bumpGlobalRevision();
}

bool AppController::exportCurrentPage(const QUrl& fileUrl, int ppi, bool jpeg, int quality)
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_document.pages.size())
        return false;
    const QImage image = renderPageAt(m_currentPageIndex, ppi);
    if (image.isNull()) {
        setStatus(QStringLiteral("导出失败：渲染为空"));
        return false;
    }
    const QString path = fileUrl.toLocalFile();
    if (path.isEmpty()) {
        setStatus(QStringLiteral("导出失败：路径无效"));
        return false;
    }
    const bool saved = jpeg ? image.save(path, "JPEG", quality) : image.save(path, "PNG");
    setStatus(saved ? QStringLiteral("已导出 %1 · %2 PPI").arg(QFileInfo(path).fileName()).arg(ppi)
                    : QStringLiteral("导出失败：%1").arg(path));
    return saved;
}

bool AppController::exportAllPages(const QUrl& directoryUrl, int ppi, bool jpeg, int quality)
{
    const QString dirPath = directoryUrl.toLocalFile();
    QDir dir(dirPath);
    if (!dir.exists()) {
        setStatus(QStringLiteral("导出失败：目录不存在"));
        return false;
    }
    int exported = 0;
    for (int i = 0; i < m_document.pages.size(); ++i) {
        const QImage image = renderPageAt(i, ppi);
        if (image.isNull())
            continue;
        const QString name = QStringLiteral("liusu-PG-%1.%2")
                                 .arg(i + 1, 3, 10, QLatin1Char('0'))
                                 .arg(jpeg ? QStringLiteral("jpg") : QStringLiteral("png"));
        const QString path = dir.filePath(name);
        const bool saved = jpeg ? image.save(path, "JPEG", quality) : image.save(path, "PNG");
        if (saved)
            ++exported;
    }
    setStatus(QStringLiteral("已导出 %1 页").arg(exported));
    return exported > 0;
}

bool AppController::saveProject(const QUrl& fileUrl)
{
    const QString path = fileUrl.toLocalFile();
    if (path.isEmpty())
        return false;
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        setStatus(QStringLiteral("保存失败：无法写入"));
        return false;
    }
    file.write(liusu::domain::serializeProject(m_document));
    if (!file.commit()) {
        setStatus(QStringLiteral("保存失败：提交中断"));
        return false;
    }
    setStatus(QStringLiteral("已保存 %1").arg(QFileInfo(path).fileName()));
    return true;
}

bool AppController::openProject(const QUrl& fileUrl)
{
    QFile file(fileUrl.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        setStatus(QStringLiteral("打开失败：无法读取"));
        return false;
    }
    const auto result = liusu::domain::parseProject(file.readAll());
    if (!result.ok) {
        setStatus(QStringLiteral("打开失败：%1").arg(result.errorMessage));
        return false;
    }
    m_document = result.document;
    m_currentPageIndex = 0;
    m_selectedSlot = -1;
    emit selectionChanged();
    emit currentPageChanged();
    bumpGlobalRevision();
    setStatus(QStringLiteral("已打开 %1").arg(QFileInfo(fileUrl.toLocalFile()).fileName()));
    return true;
}

ProjectPage* AppController::currentPage()
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_document.pages.size())
        return nullptr;
    return &m_document.pages[m_currentPageIndex];
}

const ProjectPage* AppController::currentPage() const
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_document.pages.size())
        return nullptr;
    return &m_document.pages[m_currentPageIndex];
}

void AppController::bumpCurrentPageRevision()
{
    if (m_currentPageIndex >= 0 && m_currentPageIndex < m_pageRevisions.size())
        ++m_pageRevisions[m_currentPageIndex];
    // 只清当前页旧缓存；其它页缓存保留（一次编辑不触发全页重渲染）。
    const QString prefix = QStringLiteral("%1|").arg(m_currentPageIndex);
    const auto keys = m_pageCache.keys();
    for (const QString& key : keys) {
        if (key.startsWith(prefix))
            m_pageCache.remove(key);
    }
    emit changed();
}

void AppController::bumpGlobalRevision()
{
    ++m_globalRevision;
    rebuildPageRevisions();
    m_pageCache.clear();
    emit changed();
}

void AppController::rebuildPageRevisions()
{
    // 页数变化时对齐修订号数量；已有页的修订号保持不变。
    const int count = m_document.pages.size();
    if (m_pageRevisions.size() > count)
        m_pageRevisions.resize(count);
    while (m_pageRevisions.size() < count)
        m_pageRevisions.append(0);
}

void AppController::setStatus(const QString& message)
{
    if (m_statusMessage == message)
        return;
    m_statusMessage = message;
    emit statusMessageChanged();
}

bool AppController::profileWidthHeight(qreal* outWidthMm, qreal* outHeightMm) const
{
    const auto profile = Profiles::findPageProfile(m_document.pageProfileId);
    if (!profile) {
        // 档案缺失属于编程/数据错误：回退 6 寸横版，保证界面仍可用。
        if (outWidthMm)
            *outWidthMm = 148.0;
        if (outHeightMm)
            *outHeightMm = 100.0;
        return false;
    }
    if (outWidthMm)
        *outWidthMm = profile->widthMm;
    if (outHeightMm)
        *outHeightMm = profile->heightMm;
    return true;
}

QImage AppController::renderPageAt(int pageIndex, int ppi) const
{
    if (pageIndex < 0 || pageIndex >= m_document.pages.size())
        return {};

    qreal widthMm = 0.0;
    qreal heightMm = 0.0;
    profileWidthHeight(&widthMm, &heightMm);
    if (widthMm <= 0.0 || heightMm <= 0.0)
        return {};

    // 页面预览缓存（图片与缓存规则·缓存分层）：key = 页号 | ppi | 逐页修订+全局修订。
    // 主预览与缩略图在同一 (页, 尺寸, 修订) 内只渲染一次；
    // 修订号变化时旧 key 自然失配，配合上面的定点清理避免全量重渲染。
    const int rev = m_pageRevisions.value(pageIndex, 0) + m_globalRevision;
    const QString key = QStringLiteral("%1|%2|%3").arg(pageIndex).arg(ppi).arg(rev);
    if (const QImage* hit = m_pageCache.object(key))
        return *hit;

    const QImage rendered = PageRenderer::renderPage(m_document.pages.at(pageIndex),
                                                     m_document.background,
                                                     widthMm, heightMm, ppi);
    if (!rendered.isNull()) {
        const qint64 cost = rendered.sizeInBytes();
        m_pageCache.insert(key, new QImage(rendered),
                           static_cast<int>(qMin<qint64>(cost, kPageCacheCostBytes)));
    }
    return rendered;
}
