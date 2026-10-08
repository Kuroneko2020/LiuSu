#pragma once
#include "ProjectDocument.h"

namespace liusu::domain {
ProjectPage emptyPage(const LayoutModel& layout);
// 第一页保留原槽位顺序与编辑状态；缩减后有照片的溢出槽位按顺序分装后续页。
// 返回值不包含项目中其他页，调用方只替换被编辑的那一页。
QList<ProjectPage> retemplatePage(const ProjectPage& page, const LayoutModel& layout);
}
