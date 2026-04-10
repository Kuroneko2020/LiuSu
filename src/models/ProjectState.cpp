#include "models/ProjectState.h"

namespace pte {

bool PageState::isValid() const
{
    for (const auto &slot : slots) {
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

void ProjectState::createPage(TemplateType templateType)
{
    PageState page;
    page.templateType = templateType;
    page.slots.resize(slotCount(templateType));
    m_pages.append(page);
    m_currentPageIndex = m_pages.size() - 1;
    emit pagesChanged();
    emit currentPageChanged();
    emit slotsChanged();
}

void ProjectState::deleteCurrentPage()
{
    if (m_currentPageIndex < 0 || m_currentPageIndex >= m_pages.size()) {
        return;
    }
    m_pages.remove(m_currentPageIndex);
    if (m_pages.isEmpty()) {
        m_currentPageIndex = -1;
    } else if (m_currentPageIndex >= m_pages.size()) {
        m_currentPageIndex = m_pages.size() - 1;
    }
    emit pagesChanged();
    emit currentPageChanged();
    emit slotsChanged();
}

void ProjectState::switchToPage(int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= m_pages.size() || pageIndex == m_currentPageIndex) {
        return;
    }
    m_currentPageIndex = pageIndex;
    emit currentPageChanged();
    emit slotsChanged();
}

int ProjectState::currentTemplateSlotCount() const
{
    const auto *page = currentPage();
    return page ? page->slots.size() : 0;
}

bool ProjectState::slotHasImage(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return false;
    }
    return page->slots.at(slotIndex).hasImage;
}

bool ProjectState::slotSelected(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return false;
    }
    return page->slots.at(slotIndex).selected;
}

QString ProjectState::slotImageLabel(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return QString();
    }
    const auto &slot = page->slots.at(slotIndex);
    return slot.hasImage ? QStringLiteral("图片 %1").arg(slotIndex + 1) : QString();
}

void ProjectState::selectSlot(int slotIndex)
{
    auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return;
    }

    for (int i = 0; i < page->slots.size(); ++i) {
        page->slots[i].selected = (i == slotIndex);
    }
    emit slotsChanged();
}

void ProjectState::assignImageToSlot(int slotIndex, const QString &path)
{
    auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return;
    }

    auto &slot = page->slots[slotIndex];
    slot.hasImage = true;
    slot.imagePath = path;
    selectSlot(slotIndex);

    emit pagesChanged();
    emit slotsChanged();
}

void ProjectState::rotateSelectedSlot90()
{
    auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return;
    }
    page->slots[index].rotation = (page->slots[index].rotation + 90) % 360;
    emit slotsChanged();
}

void ProjectState::mirrorSelectedSlot()
{
    auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return;
    }
    page->slots[index].mirrored = !page->slots[index].mirrored;
    emit slotsChanged();
}

void ProjectState::toggleSelectedSlotFillMode()
{
    auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return;
    }
    auto &slot = page->slots[index];
    slot.fillMode = slot.fillMode == FillMode::FitInside ? FillMode::FillCrop : FillMode::FitInside;
    emit slotsChanged();
}

bool ProjectState::selectedSlotInFillCrop() const
{
    const auto *page = currentPage();
    const int index = selectedSlotIndex();
    if (!page || index < 0) {
        return false;
    }
    return page->slots.at(index).fillMode == FillMode::FillCrop;
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
    for (int i = 0; i < page->slots.size(); ++i) {
        if (page->slots.at(i).selected) {
            return i;
        }
    }
    return -1;
}

} // namespace pte
