#include "ExportOptions.h"
#include <cmath>

namespace liusu::domain {
QVariantList exportOptionDefinitions()
{
    return {
        QVariantMap{{"id","format"},{"label",QStringLiteral("文件格式")},{"type","choice"},{"group",QStringLiteral("文件")},
            {"choices",QVariantList{QVariantMap{{"value","jpeg"},{"label","JPEG"}},QVariantMap{{"value","png"},{"label","PNG"}}}}},
        QVariantMap{{"id","ppi"},{"label",QStringLiteral("输出密度 · PPI")},{"type","integer"},{"group",QStringLiteral("图像")},{"min",72},{"max",1200},
            {"presets",QVariantList{300,600}},{"help",QStringLiteral("改变像素密度，相纸与槽位的物理尺寸保持一致。")}},
        QVariantMap{{"id","jpegQuality"},{"label",QStringLiteral("JPEG 质量")},{"type","integer"},{"group",QStringLiteral("图像")},{"min",1},{"max",100},{"advanced",true},
            {"visibleWhen",QVariantMap{{"id","format"},{"value","jpeg"}}}}
    };
}
QVariantMap exportOptionValues(const ExportSettings& settings)
{
    return {{"format",settings.jpegFormat ? "jpeg" : "png"},{"ppi",settings.ppi},{"jpegQuality",settings.jpegQuality}};
}
bool setExportOption(ExportSettings& settings,const QString& id,const QVariant& value,QString* error)
{
    auto fail=[&] { if(error) *error=QStringLiteral("不支持的导出设置或参数：%1").arg(id); return false; };
    if(id=="format") {
        if(value.metaType().id()!=QMetaType::QString || (value.toString()!="jpeg" && value.toString()!="png")) return fail();
        settings.jpegFormat=value.toString()=="jpeg";
    } else if(id=="ppi" || id=="jpegQuality") {
        const auto type=value.metaType().id();
        if(type!=QMetaType::Int && type!=QMetaType::Double && type!=QMetaType::LongLong && type!=QMetaType::UInt) return fail();
        const double number=value.toDouble();
        const int minimum=id=="ppi" ? 72 : 1, maximum=id=="ppi" ? 1200 : 100;
        if(!std::isfinite(number) || std::floor(number)!=number || number<minimum || number>maximum) return fail();
        if(id=="ppi") settings.ppi=static_cast<int>(number);
        else settings.jpegQuality=static_cast<int>(number);
    } else return fail();
    if(error) error->clear();
    return true;
}
}
