#include "PageOperations.h"

namespace liusu::domain {
ProjectPage emptyPage(const LayoutModel& layout,const QString& templateId)
{
    ProjectPage page;
    page.layout = layout;
    page.templateId = templateId;
    for (int i = 0; i < layout.slotRects.size(); ++i) page.slotStates.append(SlotImageState{});
    return page;
}

QList<ProjectPage> retemplatePage(const ProjectPage& page, const LayoutModel& layout,const QString& templateId)
{
    if (!layout.isValid()) return {};
    const int capacity = layout.slotRects.size();
    QList<ProjectPage> result { emptyPage(layout,templateId) };
    for (int i = 0; i < qMin(capacity, page.slotStates.size()); ++i)
        result[0].slotStates[i] = page.slotStates[i];
    int next = capacity;
    for (int i = capacity; i < page.slotStates.size(); ++i) {
        const auto& state = page.slotStates[i];
        if (state.imagePath.isEmpty()) continue;
        if (next == capacity) {
            result.append(emptyPage(layout,templateId));
            next = 0;
        }
        result.last().slotStates[next++] = state;
    }
    return result;
}
}
