#include "models/ProjectState.h"

#include "services/TemplateLayout.h"
#include <QUrl>

namespace pte {

bool PageState::isValid() const
{
    for (const auto &slot : slotStates) {
        if (slot.hasImage) {
            return true;
        }
    }
    return false;
}

ProjectState::ProjectState(QObject *parent)
    : QObject(parent)
{
}

void ProjectState::ensureInitialPage(TemplateType templateType)
{
    if (!m_pages.isEmpty()) {
        return;
    }
    createPage(templateType);
}

void ProjectState::startNewSession(TemplateType templateType)
{
    m_pages.clear();
    m_currentPageIndex = -1;
    createPage(templateType);
}

void ProjectState::createPage(TemplateType templateType)
{
    PageState page;
    page.templateType = templateType;
    page.slotStates.resize(slotCount(templateType));
    m_pages.append(page);
    m_currentPageIndex = m_pages.size() - 1;
    emit pagesChanged();
    emit currentPageChanged();
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::deleteCurrentPage()
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_pages.size()) {
        return;
    }
    const TemplateType removedType = m_pages.at(m_currentPageIndex).templateType;
    m_pages.remove(m_currentPageIndex);
    if (m_pages.isEmpty()) {
        PageState page;
        page.templateType = removedType;
        page.slotStates.resize(slotCount(removedType));
        m_pages.append(page);
        m_currentPageIndex = 0;
    } else if (m_currentPageIndex >= m_pages.size()) {
        m_currentPageIndex = m_pages.size() - 1;
    }
    emit pagesChanged();
    emit currentPageChanged();
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::switchToPage(int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= m_pages.size() || pageIndex == m_currentPageIndex) {
        return;
    }
    m_currentPageIndex = pageIndex;
    emit currentPageChanged();
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

int ProjectState::currentTemplateSlotCount() const
{
    const auto *page = currentPage();
    return page ? page->slotStates.size() : 0;
}

int ProjectState::currentTemplateChoice() const
{
    const auto *page = currentPage();
    return page ? static_cast<int>(page->templateType) : 2;
}

bool ProjectState::slotHasImage(int slotIndex) const
{
    return pageSlotHasImage(m_currentPageIndex, slotIndex);
}

bool ProjectState::slotSelected(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size()) {
        return false;
    }
    return page->slotStates.at(slotIndex).selected;
}

QString ProjectState::slotImagePath(int slotIndex) const
{
    return pageSlotImagePath(m_currentPageIndex, slotIndex);
}

QString ProjectState::slotImageSource(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size()) {
        return {};
    }
    const auto &image = page->slotStates.at(slotIndex).image;
    const QString path = image.previewPath.isEmpty() ? image.exportPath : image.previewPath;
    if (path.isEmpty()) {
        return {};
    }
    return QUrl::fromLocalFile(path).toString();
}

QString ProjectState::slotOriginalBaseName(int slotIndex) const
{
    return pageSlotOriginalBaseName(m_currentPageIndex, slotIndex);
}

int ProjectState::slotRotation(int slotIndex) const
{
    return pageSlotRotation(m_currentPageIndex, slotIndex);
}

bool ProjectState::slotMirrored(int slotIndex) const
{
    return pageSlotMirrored(m_currentPageIndex, slotIndex);
}

bool ProjectState::slotFillCrop(int slotIndex) const
{
    return pageSlotFillCrop(m_currentPageIndex, slotIndex);
}

qreal ProjectState::slotOffsetX(int slotIndex) const
{
    return pageSlotOffset(m_currentPageIndex, slotIndex).x();
}

qreal ProjectState::slotOffsetY(int slotIndex) const
{
    return pageSlotOffset(m_currentPageIndex, slotIndex).y();
}

QRectF ProjectState::slotRectNormalized(int slotIndex) const
{
    return pageSlotRectNormalized(m_currentPageIndex, slotIndex);
}

void ProjectState::selectSlot(int slotIndex)
{
    auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size()) {
        return;
    }

    for (int i = 0; i < page->slotStates.size(); ++i) {
        page->slotStates[i].selected = (i == slotIndex);
    }
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::clearSelection()
{
    auto *page = currentPage();
    if (!page) {
        return;
    }
    bool changed = false;
    for (auto &slot : page->slotStates) {
        if (slot.selected) {
            slot.selected = false;
            changed = true;
        }
    }
    if (!changed) {
        return;
    }
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::assignImageToSlot(int slotIndex, const pte::ImageResource &resource)
{
    auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size()) {
        return;
    }

    auto &slot = page->slotStates[slotIndex];
    slot = SlotState{};
    slot.hasImage = true;
    slot.image = resource;
    if (!slot.selected) {
        selectSlot(slotIndex);
    }

    emit pagesChanged();
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::configureSlot(int slotIndex, bool fillCrop, int rotation, bool mirrored)
{
    auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slotStates.size() || !page->slotStates[slotIndex].hasImage) {
        return;
    }
    auto &slot = page->slotStates[slotIndex];
    slot.fillMode = fillCrop ? FillMode::FillCrop : FillMode::FitInside;
    slot.rotation = ((rotation % 360) + 360) % 360;
    slot.mirrored = mirrored;
    slot.cropOffset = QPointF(0.0, 0.0);
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::rotateSelectedSlot90()
{
    auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return;
    }
    page->slotStates[index].rotation = (page->slotStates[index].rotation + 90) % 360;
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::mirrorSelectedSlot()
{
    auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return;
    }
    page->slotStates[index].mirrored = !page->slotStates[index].mirrored;
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::toggleSelectedSlotFillMode()
{
    auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return;
    }
    auto &slot = page->slotStates[index];
    slot.fillMode = slot.fillMode == FillMode::FitInside ? FillMode::FillCrop : FillMode::FitInside;
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

bool ProjectState::selectedSlotInFillCrop() const
{
    const auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return false;
    }
    return page->slotStates.at(index).fillMode == FillMode::FillCrop;
}

void ProjectState::adjustSelectedSlotOffset(qreal dx, qreal dy)
{
    auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return;
    }
    auto &slot = page->slotStates[index];
    if (slot.fillMode != FillMode::FillCrop || !slot.hasImage) {
        return;
    }
    slot.cropOffset.setX(qBound(-1.0, slot.cropOffset.x() + dx, 1.0));
    slot.cropOffset.setY(qBound(-1.0, slot.cropOffset.y() + dy, 1.0));
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

void ProjectState::swapOrMoveSlots(int fromIndex, int toIndex)
{
    auto *page = currentPage();
    if (!page || fromIndex < 0 || toIndex < 0 || fromIndex >= page->slotStates.size() || toIndex >= page->slotStates.size() || fromIndex == toIndex) {
        return;
    }

    if (page->slotStates[toIndex].hasImage) {
        qSwap(page->slotStates[fromIndex], page->slotStates[toIndex]);
    } else {
        page->slotStates[toIndex] = page->slotStates[fromIndex];
        page->slotStates[fromIndex] = SlotState{};
    }

    for (int i = 0; i < page->slotStates.size(); ++i) {
        page->slotStates[i].selected = (i == toIndex);
    }

    emit pagesChanged();
    ++m_contentRevision;
    ++m_slotsRevision;
    emit slotsChanged();
}

int ProjectState::findNextAvailableSlot() const
{
    const auto *page = currentPage();
    if (!page) {
        return -1;
    }

    const int selectedIndex = selectedSlotIndex();
    if (selectedIndex >= 0 && selectedIndex < page->slotStates.size() && !page->slotStates.at(selectedIndex).hasImage) {
        return selectedIndex;
    }

    for (int i = 0; i < page->slotStates.size(); ++i) {
        if (!page->slotStates.at(i).hasImage) {
            return i;
        }
    }
    return -1;
}


int ProjectState::visiblePageCount() const
{
    int count = 0;
    for (int i = 0; i < m_pages.size(); ++i) {
        if (m_pages.at(i).isValid() || i == m_currentPageIndex) {
            ++count;
        }
    }
    return count;
}

int ProjectState::visiblePageIndexAt(int visibleIndex) const
{
    int cursor = 0;
    for (int i = 0; i < m_pages.size(); ++i) {
        if (!(m_pages.at(i).isValid() || i == m_currentPageIndex)) {
            continue;
        }
        if (cursor == visibleIndex) {
            return i;
        }
        ++cursor;
    }
    return -1;
}

int ProjectState::pageTemplateChoice(int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return 2;
    }
    return static_cast<int>(m_pages.at(pageIndex).templateType);
}

int ProjectState::pageSlotCount(int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return 0;
    }
    return m_pages.at(pageIndex).slotStates.size();
}

bool ProjectState::pageSlotHasImage(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return false;
    }
    const auto &slotStates = m_pages.at(pageIndex).slotStates;
    if (slotIndex < 0 || slotIndex >= slotStates.size()) {
        return false;
    }
    return slotStates.at(slotIndex).hasImage;
}

QString ProjectState::pageSlotImagePath(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return {};
    }
    const auto &slotStates = m_pages.at(pageIndex).slotStates;
    if (slotIndex < 0 || slotIndex >= slotStates.size()) {
        return {};
    }
    return slotStates.at(slotIndex).image.exportPath;
}

QString ProjectState::pageSlotOriginalBaseName(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return {};
    }
    const auto &slotStates = m_pages.at(pageIndex).slotStates;
    if (slotIndex < 0 || slotIndex >= slotStates.size()) {
        return {};
    }
    return slotStates.at(slotIndex).image.originalBaseName;
}

int ProjectState::pageSlotRotation(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return 0;
    }
    const auto &slotStates = m_pages.at(pageIndex).slotStates;
    if (slotIndex < 0 || slotIndex >= slotStates.size()) {
        return 0;
    }
    return slotStates.at(slotIndex).rotation;
}

bool ProjectState::pageSlotMirrored(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return false;
    }
    const auto &slotStates = m_pages.at(pageIndex).slotStates;
    if (slotIndex < 0 || slotIndex >= slotStates.size()) {
        return false;
    }
    return slotStates.at(slotIndex).mirrored;
}

bool ProjectState::pageSlotFillCrop(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return false;
    }
    const auto &slotStates = m_pages.at(pageIndex).slotStates;
    if (slotIndex < 0 || slotIndex >= slotStates.size()) {
        return false;
    }
    return slotStates.at(slotIndex).fillMode == FillMode::FillCrop;
}

QPointF ProjectState::pageSlotOffset(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return {};
    }
    const auto &slotStates = m_pages.at(pageIndex).slotStates;
    if (slotIndex < 0 || slotIndex >= slotStates.size()) {
        return {};
    }
    return slotStates.at(slotIndex).cropOffset;
}

QRectF ProjectState::pageSlotRectNormalized(int pageIndex, int slotIndex) const
{
    const auto rects = layout::slotRectsNormalized(pageTemplateChoice(pageIndex));
    if (slotIndex < 0 || slotIndex >= rects.size()) {
        return {};
    }
    return rects.at(slotIndex);
}

int ProjectState::currentPageIndex() const
{
    return m_currentPageIndex;
}

int ProjectState::pageCount() const
{
    return m_pages.size();
}

bool ProjectState::isPageValid(int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return false;
    }
    return m_pages.at(pageIndex).isValid();
}

bool ProjectState::hasValidPages() const
{
    for (const auto &page : m_pages) {
        if (page.isValid()) {
            return true;
        }
    }
    return false;
}

int ProjectState::slotsRevision() const
{
    return m_slotsRevision;
}

int ProjectState::contentRevision() const
{
    return m_contentRevision;
}

PageState *ProjectState::currentPage()
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_pages.size()) {
        return nullptr;
    }
    return &m_pages[m_currentPageIndex];
}

const PageState *ProjectState::currentPage() const
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_pages.size()) {
        return nullptr;
    }
    return &m_pages[m_currentPageIndex];
}

int ProjectState::selectedSlotIndex() const
{
    const auto *page = currentPage();
    if (!page) {
        return -1;
    }
    for (int i = 0; i < page->slotStates.size(); ++i) {
        if (page->slotStates.at(i).selected) {
            return i;
        }
    }
    return -1;
}

} // namespace pte
