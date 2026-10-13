#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QCoreApplication>
#include "component_versions.h"
#include "mainwindow.h"

// Fail closed on missing, unreadable, or out-of-date private version manifest.
// This does not load or inspect the protected question collection.
static bool localGuardEnabled() {
    QFile file(QDir::homePath() + "/.local/state/recallforge/settings.json");
    if (!file.open(QIODevice::ReadOnly) || file.size()>65536) return true;
    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error!=QJsonParseError::NoError || !doc.isObject()) return true;
    return doc.object().value("version_checks").toObject().value("local_enabled").toBool(true);
}
static QString validateVersions() {
    const QString file = QDir::homePath() + "/.local/state/recallforge/ver.json";
    QFile input(file);
    const QJsonObject expected = QJsonDocument::fromJson(QByteArray(kRecallForgeVersions)).object();
    QJsonObject found;
    QStringList errors;
    if (input.open(QIODevice::ReadOnly)) {
        QJsonParseError parse;
        auto document=QJsonDocument::fromJson(input.readAll(), &parse);
        if (parse.error == QJsonParseError::NoError && document.isObject())
            found=document.object().value("components").toObject();
        else errors << "Version file is invalid JSON or missing components.";
    } else errors << "Version file is missing or unreadable.";
    for (auto it=expected.begin();it!=expected.end();++it) {
        const QString actual=found.value(it.key()).toString("missing");
        if (actual!=it.value().toString())
            errors << QString("%1: found %2; required %3").arg(it.key(),actual,it.value().toString());
    }
    if (errors.isEmpty()) return {};
    return "Chatgpt, the following version file is " + file +
           " and the parts that are out of date and will become a problem are:\n" +
           errors.join("\n") +
           "\n\nReinstall the matching RecallForge source package and installer. Do not edit ver.json manually.";
}
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("RecallForge");
    const QString failure=localGuardEnabled() ? validateVersions() : QString();
    if (!failure.isEmpty()) {
        QTextStream(stderr) << failure << Qt::endl;
        if (!QCoreApplication::arguments().contains("--version-check"))
            QMessageBox::critical(nullptr,"RecallForge version mismatch",failure);
        return 78;
    }
    if (QCoreApplication::arguments().contains("--version-check")) {
        QTextStream(stdout) << (localGuardEnabled() ? "RecallForge components match ver.json" : "RecallForge local version check disabled in settings.json") << Qt::endl;
        return 0;
    }
    MainWindow window; window.show(); return app.exec();
}
