#pragma once

#include "models/Types.h"
#include "models/ImageResource.h"

#include <QObject>
#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QVector>

namespace pte {
class ImageService;

struct SlotState {
    bool hasImage = false;
    ImageResource image;
    bool selected = false;
    int rotation = 0;
    bool mirrored = false;
    FillMode fillMode = FillMode::FitInside;
    QPointF cropOffset = QPointF(0.0, 0.0);
    qreal zoom = 1.0;
};

struct PageState {
    TemplateType templateType = TemplateType::TwoUp;
    QVector<SlotState> slotStates;

    [[nodiscard]] bool isValid() const;
};

class ProjectState : public QObject {
    Q_OBJECT
    Q_PROPERTY(int currentPageIndex READ currentPageIndex NOTIFY currentPageChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY pagesChanged)
    Q_PROPERTY(int currentTemplateSlotCount READ currentTemplateSlotCount NOTIFY slotsChanged)
    Q_PROPERTY(int currentTemplateChoice READ currentTemplateChoice NOTIFY currentPageChanged)
    Q_PROPERTY(int slotsRevision READ slotsRevision NOTIFY slotsChanged)
    Q_PROPERTY(int contentRevision READ contentRevision NOTIFY slotsChanged)
    Q_PROPERTY(QString backgroundMode READ backgroundMode WRITE setBackgroundMode NOTIFY backgroundChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundChanged)
    Q_PROPERTY(QString backgroundTexturePath READ backgroundTexturePath WRITE setBackgroundTexturePath NOTIFY backgroundChanged)
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
    Q_INVOKABLE QString slotImageSource(int slotIndex) const;
    Q_INVOKABLE QString slotOriginalBaseName(int slotIndex) const;
    Q_INVOKABLE int slotRotation(int slotIndex) const;
    Q_INVOKABLE bool slotMirrored(int slotIndex) const;
    Q_INVOKABLE bool slotFillCrop(int slotIndex) const;
    Q_INVOKABLE qreal slotOffsetX(int slotIndex) const;
    Q_INVOKABLE qreal slotOffsetY(int slotIndex) const;
    Q_INVOKABLE QRectF slotRectNormalized(int slotIndex) const;

    Q_INVOKABLE void selectSlot(int slotIndex);
    Q_INVOKABLE void clearSelection();
    void setImageService(ImageService *imageService);
    void assignImageToSlot(int slotIndex, const pte::ImageResource &resource);
    void refreshSlotPreviewResources();
    void configureSlot(int slotIndex, bool fillCrop, int rotation, bool mirrored);
    Q_INVOKABLE void rotateSelectedSlot90();
    Q_INVOKABLE void mirrorSelectedSlot();
    Q_INVOKABLE void toggleSelectedSlotFillMode();
    Q_INVOKABLE bool selectedSlotInFillCrop() const;
    Q_INVOKABLE void adjustSelectedSlotOffset(qreal dx, qreal dy);
    Q_INVOKABLE void setSelectedSlotOffset(qreal x, qreal y);
    Q_INVOKABLE void resetSelectedSlotOffset();
    Q_INVOKABLE void clearSlot(int slotIndex);
    Q_INVOKABLE void swapOrMoveSlots(int fromIndex, int toIndex);

    Q_INVOKABLE int findNextAvailableSlot() const;

    [[nodiscard]] int currentPageIndex() const;
    [[nodiscard]] int pageCount() const;
    [[nodiscard]] bool isPageValid(int pageIndex) const;
    Q_INVOKABLE bool hasValidPages() const;
    [[nodiscard]] int slotsRevision() const;
    [[nodiscard]] int contentRevision() const;
    [[nodiscard]] QString backgroundMode() const;
    void setBackgroundMode(const QString &mode);
    [[nodiscard]] QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);
    [[nodiscard]] QString backgroundTexturePath() const;
    void setBackgroundTexturePath(const QString &path);

    [[nodiscard]] int pageTemplateChoice(int pageIndex) const;
    [[nodiscard]] int pageSlotCount(int pageIndex) const;
    [[nodiscard]] bool pageSlotHasImage(int pageIndex, int slotIndex) const;
    [[nodiscard]] QString pageSlotImagePath(int pageIndex, int slotIndex) const;
    [[nodiscard]] QString pageSlotPreviewPath(int pageIndex, int slotIndex) const;
    [[nodiscard]] QString pageSlotOriginalBaseName(int pageIndex, int slotIndex) const;
    [[nodiscard]] int pageSlotRotation(int pageIndex, int slotIndex) const;
    [[nodiscard]] bool pageSlotMirrored(int pageIndex, int slotIndex) const;
    [[nodiscard]] bool pageSlotFillCrop(int pageIndex, int slotIndex) const;
    [[nodiscard]] QPointF pageSlotOffset(int pageIndex, int slotIndex) const;
    [[nodiscard]] QRectF pageSlotRectNormalized(int pageIndex, int slotIndex) const;

signals:
    void pagesChanged();
    void currentPageChanged();
    void slotsChanged();
    void backgroundChanged();

private:
    [[nodiscard]] PageState *currentPage();
    [[nodiscard]] const PageState *currentPage() const;
    [[nodiscard]] int selectedSlotIndex() const;

    QVector<PageState> m_pages;
    ImageService *m_imageService = nullptr;
    int m_currentPageIndex = -1;
    int m_slotsRevision = 0;
    int m_contentRevision = 0;
    QString m_backgroundMode = QStringLiteral("color");
    QColor m_backgroundColor = QColor(QStringLiteral("#F7F4EC"));
    QString m_backgroundTexturePath;
};

} // namespace pte
