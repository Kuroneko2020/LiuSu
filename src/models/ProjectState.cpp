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

QString PageState::previewImagePath() const
{
    for (const auto &slot : slots) {
        if (slot.hasImage) {
            return slot.imagePath;
        }
    }
    return {};
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
    const TemplateType removedType = m_pages.at(m_currentPageIndex).templateType;
    m_pages.remove(m_currentPageIndex);
    if (m_pages.isEmpty()) {
        PageState page;
        page.templateType = removedType;
        page.slots.resize(slotCount(removedType));
        m_pages.append(page);
        m_currentPageIndex = 0;
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

int ProjectState::currentTemplateChoice() const
{
    const auto *page = currentPage();
    return page ? static_cast<int>(page->templateType) : 2;
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

QString ProjectState::slotImagePath(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return {};
    }
    return page->slots.at(slotIndex).imagePath;
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

int ProjectState::slotRotation(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return 0;
    }
    return page->slots.at(slotIndex).rotation;
}

bool ProjectState::slotMirrored(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return false;
    }
    return page->slots.at(slotIndex).mirrored;
}

bool ProjectState::slotFillCrop(int slotIndex) const
{
    const auto *page = currentPage();
    if (!page || slotIndex < 0 || slotIndex >= page->slots.size()) {
        return false;
    }
    return page->slots.at(slotIndex).fillMode == FillMode::FillCrop;
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
    if (!slot.selected) {
        selectSlot(slotIndex);
    }

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

int ProjectState::findNextAvailableSlot() const
{
    const auto *page = currentPage();
    if (!page) {
        return -1;
    }

    const int selectedIndex = selectedSlotIndex();
    if (selectedIndex >= 0 && selectedIndex < page->slots.size() && !page->slots.at(selectedIndex).hasImage) {
        return selectedIndex;
    }

    for (int i = 0; i < page->slots.size(); ++i) {
        if (!page->slots.at(i).hasImage) {
            return i;
        }
    }
    return -1;
}

QString ProjectState::pagePreviewImagePath(int pageIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return {};
    }
    return m_pages.at(pageIndex).previewImagePath();
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
    return m_pages.at(pageIndex).slots.size();
}

bool ProjectState::pageSlotHasImage(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return false;
    }
    const auto &slots = m_pages.at(pageIndex).slots;
    if (slotIndex < 0 || slotIndex >= slots.size()) {
        return false;
    }
    return slots.at(slotIndex).hasImage;
}

QString ProjectState::pageSlotImagePath(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return {};
    }
    const auto &slots = m_pages.at(pageIndex).slots;
    if (slotIndex < 0 || slotIndex >= slots.size()) {
        return {};
    }
    return slots.at(slotIndex).imagePath;
}

int ProjectState::pageSlotRotation(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return 0;
    }
    const auto &slots = m_pages.at(pageIndex).slots;
    if (slotIndex < 0 || slotIndex >= slots.size()) {
        return 0;
    }
    return slots.at(slotIndex).rotation;
}

bool ProjectState::pageSlotMirrored(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return false;
    }
    const auto &slots = m_pages.at(pageIndex).slots;
    if (slotIndex < 0 || slotIndex >= slots.size()) {
        return false;
    }
    return slots.at(slotIndex).mirrored;
}

bool ProjectState::pageSlotFillCrop(int pageIndex, int slotIndex) const
{
    if (pageIndex < 0 || pageIndex >= m_pages.size()) {
        return false;
    }
    const auto &slots = m_pages.at(pageIndex).slots;
    if (slotIndex < 0 || slotIndex >= slots.size()) {
        return false;
    }
    return slots.at(slotIndex).fillMode == FillMode::FillCrop;
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
