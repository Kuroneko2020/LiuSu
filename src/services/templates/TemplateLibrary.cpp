#include "TemplateLibrary.h"
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>
#include <QCryptographicHash>

namespace liusu::services {
TemplateLibrary::TemplateLibrary(const QString& directory)
    : m_directory(directory.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/templates" : directory)
{
    const QDir folder(m_directory);
    for(const auto& name:folder.entryList({"*.json"},QDir::Files,QDir::Name)) {
        QFile file(folder.filePath(name));
        QString error;
        if(!file.open(QIODevice::ReadOnly) || !m_catalog.appendFromJson(file.readAll(),&error))
            m_loadError=QStringLiteral("部分模板目录未加载：%1 %2").arg(name,error);
    }
}
bool TemplateLibrary::importCatalog(const QUrl& fileUrl,QString* error)
{
    auto fail=[&](const QString& message) { if(error) *error=message; return false; };
    QFile input(fileUrl.toLocalFile());
    if(!fileUrl.isLocalFile() || !input.open(QIODevice::ReadOnly)) return fail(QStringLiteral("无法读取本地模板目录"));
    const auto bytes=input.readAll();
    auto candidate=m_catalog;
    if(!candidate.appendFromJson(bytes,error)) return false;
    if(!QDir().mkpath(m_directory)) return fail(QStringLiteral("无法建立模板存储目录"));
    // 保存输入快照而非外部路径，来源文件移动/删除不影响已导入模板。
    const QString name=QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex())+".json";
    QSaveFile output(QDir(m_directory).filePath(name));
    if(!output.open(QIODevice::WriteOnly) || output.write(bytes)!=bytes.size() || !output.commit())
        return fail(QStringLiteral("模板目录保存失败，未加入模板库"));
    m_catalog=std::move(candidate);
    return true;
}
}
