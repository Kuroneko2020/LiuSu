#pragma once

#include "models/Types.h"

#include <QObject>
#include <QPointF>
#include <QVector>

namespace pte {

struct SlotState {
    bool hasImage = false;
    QString imagePath;
    bool selected = false;
    int rotation = 0;
    bool mirrored = false;
    FillMode fillMode = FillMode::FitInside;
    QPointF cropOffset = QPointF(0.0, 0.0);
    qreal zoom = 1.0;
};

struct PageState {
    TemplateType templateType = TemplateType::TwoUp;
    QVector<SlotState> slots;

    [[nodiscard]] bool isValid() const;
};

class ProjectState : public QObject {
    Q_OBJECT
    Q_PROPERTY(int currentPageIndex READ currentPageIndex NOTIFY currentPageChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pagesChanged)
public:
    explicit ProjectState(QObject *parent = nullptr);

    Q_INVOKABLE void ensureInitialPage(TemplateType templateType);
    Q_INVOKABLE void startNewSession(TemplateType templateType);
    Q_INVOKABLE void createPage(TemplateType templateType);
    Q_INVOKABLE void deleteCurrentPage();
    Q_INVOKABLE void switchToPage(int pageIndex);

    Q_INVOKABLE int currentTemplateSlotCount() const;
    Q_INVOKABLE int currentTemplateChoice() const;
    Q_INVOKABLE bool slotHasImage(int slotIndex) const;
    Q_INVOKABLE bool slotSelected(int slotIndex) const;
    Q_INVOKABLE QString slotImagePath(int slotIndex) const;
    Q_INVOKABLE int slotRotation(int slotIndex) const;
    Q_INVOKABLE bool slotMirrored(int slotIndex) const;
    Q_INVOKABLE bool slotFillCrop(int slotIndex) const;
    Q_INVOKABLE qreal slotOffsetX(int slotIndex) const;
    Q_INVOKABLE qreal slotOffsetY(int slotIndex) const;
    Q_INVOKABLE QRectF slotRectNormalized(int slotIndex) const;

    Q_INVOKABLE void selectSlot(int slotIndex);
    Q_INVOKABLE void assignImageToSlot(int slotIndex, const QString &path);
    Q_INVOKABLE void configureSlot(int slotIndex, bool fillCrop, int rotation, bool mirrored);
    Q_INVOKABLE void rotateSelectedSlot90();
    Q_INVOKABLE void mirrorSelectedSlot();
    Q_INVOKABLE void toggleSelectedSlotFillMode();
    Q_INVOKABLE bool selectedSlotInFillCrop() const;
    Q_INVOKABLE void adjustSelectedSlotOffset(qreal dx, qreal dy);
    Q_INVOKABLE void swapOrMoveSlots(int fromIndex, int toIndex);

    Q_INVOKABLE int findNextAvailableSlot() const;

    [[nodiscard]] int currentPageIndex() const;
    [[nodiscard]] int pageCount() const;
    [[nodiscard]] bool isPageValid(int pageIndex) const;
    [[nodiscard]] bool hasValidPages() const;

    [[nodiscard]] int pageTemplateChoice(int pageIndex) const;
    [[nodiscard]] int pageSlotCount(int pageIndex) const;
    [[nodiscard]] bool pageSlotHasImage(int pageIndex, int slotIndex) const;
    [[nodiscard]] QString pageSlotImagePath(int pageIndex, int slotIndex) const;
    [[nodiscard]] int pageSlotRotation(int pageIndex, int slotIndex) const;
    [[nodiscard]] bool pageSlotMirrored(int pageIndex, int slotIndex) const;
    [[nodiscard]] bool pageSlotFillCrop(int pageIndex, int slotIndex) const;
    [[nodiscard]] QPointF pageSlotOffset(int pageIndex, int slotIndex) const;
    [[nodiscard]] QRectF pageSlotRectNormalized(int pageIndex, int slotIndex) const;

signals:
    void pagesChanged();
    void currentPageChanged();
    void slotsChanged();

private:
    [[nodiscard]] PageState *currentPage();
    [[nodiscard]] const PageState *currentPage() const;
    [[nodiscard]] int selectedSlotIndex() const;

    QVector<PageState> m_pages;
    int m_currentPageIndex = -1;
};

} // namespace pte
