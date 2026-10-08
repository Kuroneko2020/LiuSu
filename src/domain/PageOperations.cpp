#include "PageOperations.h"

namespace liusu::domain {
ProjectPage emptyPage(const LayoutModel& layout)
{
    ProjectPage page;
    page.layout = layout;
    for (int i = 0; i < layout.slotRects.size(); ++i) page.slotStates.append(SlotImageState{});
    return page;
}

QList<ProjectPage> retemplatePage(const ProjectPage& page, const LayoutModel& layout)
{
    if (!layout.isValid()) return {};
    const int capacity = layout.slotRects.size();
    QList<ProjectPage> result { emptyPage(layout) };
    for (int i = 0; i < qMin(capacity, page.slotStates.size()); ++i)
        result[0].slotStates[i] = page.slotStates[i];
    int next = capacity;
    for (int i = capacity; i < page.slotStates.size(); ++i) {
        const auto& state = page.slotStates[i];
        if (state.imagePath.isEmpty()) continue;
        if (next == capacity) {
            result.append(emptyPage(layout));
            next = 0;
        }
        result.last().slotStates[next++] = state;
    }
    return result;
}
}
