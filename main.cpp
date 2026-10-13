#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QSaveFile>
#include <QTextStream>
#include <QStringList>
#include "component_versions.h"
#include "mainwindow.h"

static bool isFlatpak() {
    return qEnvironmentVariableIsSet("FLATPAK_ID");
}

static QString stateDirectory() {
    if (isFlatpak()) {
        const QString xdg = qEnvironmentVariable("XDG_STATE_HOME");
        if (!xdg.isEmpty() && QDir::isAbsolutePath(xdg))
            return QDir(xdg).filePath("recallforge");
        return QDir::home().filePath(
            ".var/app/org.oneal.RecallForge/.local/state/recallforge");
    }
    return QDir::home().filePath(".local/state/recallforge");
}

static bool localGuardEnabled() {
    QFile file(QDir(stateDirectory()).filePath("settings.json"));
    if (!file.open(QIODevice::ReadOnly) || file.size() > 65536)
        return true;

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError || !document.isObject())
        return true;

    return document.object()
        .value("version_checks")
        .toObject()
        .value("local_enabled")
        .toBool(true);
}

static QString initializeFlatpakManifest(const QString &path,
                                         const QJsonObject &expected) {
    if (!isFlatpak())
        return "Native installation requires its existing installer.";

    if (expected.isEmpty())
        return "Embedded component versions are unavailable.";

    QFileInfo info(path);

    if (info.exists() || info.isSymLink())
        return "Existing version manifest will not be overwritten.";

    const QString directory = QFileInfo(path).absolutePath();

    if (!QDir().mkpath(directory))
        return "Could not create Flatpak state directory.";

    if (!QFile::setPermissions(
            directory,
            QFileDevice::ReadOwner |
            QFileDevice::WriteOwner |
            QFileDevice::ExeOwner))
        return "Could not secure Flatpak state directory.";

    QJsonObject root;
    root.insert("components", expected);

    QSaveFile output(path);
    output.setPermissions(
        QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    if (!output.open(QIODevice::WriteOnly))
        return "Could not create Flatpak version manifest.";

    const QByteArray contents =
        QJsonDocument(root).toJson(QJsonDocument::Indented);

    if (output.write(contents) != contents.size() || !output.commit())
        return "Could not save Flatpak version manifest.";

    return {};
}

static QString validateVersions() {
    const QString path =
        QDir(stateDirectory()).filePath("ver.json");

    const QJsonObject expected =
        QJsonDocument::fromJson(
            QByteArray(kRecallForgeVersions)).object();

    if (expected.isEmpty())
        return "Embedded RecallForge version information is invalid.";

    QFileInfo info(path);

    if (!info.exists() && !info.isSymLink() && isFlatpak()) {
        const QString error =
            initializeFlatpakManifest(path, expected);

        if (!error.isEmpty())
            return error;
    }

    QStringList errors;
    QJsonObject found;

    QFile input(path);

    if (info.isSymLink() || QFileInfo(path).isSymLink()) {
        errors << "Version manifest must not be a symbolic link.";
    } else if (input.open(QIODevice::ReadOnly)) {
        QJsonParseError parse;
        const auto document =
            QJsonDocument::fromJson(input.readAll(), &parse);

        if (parse.error == QJsonParseError::NoError &&
            document.isObject() &&
            document.object().value("components").isObject()) {
            found = document.object().value("components").toObject();
        } else {
            errors << "Invalid version JSON or missing components object.";
        }
    } else {
        errors << "Version file is missing or unreadable.";
    }

    for (auto it = expected.begin(); it != expected.end(); ++it) {
        const QString actual =
            found.value(it.key()).toString("missing");

        if (actual != it.value().toString()) {
            errors << QString("%1: found %2; required %3")
                .arg(it.key(), actual, it.value().toString());
        }
    }

    if (errors.isEmpty())
        return {};

    return "RecallForge version validation failed:\n" +
           path + "\n\n" +
           errors.join("\n") +
           "\n\nRestore or reinstall the matching application package.";
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("RecallForge");

    const bool check = localGuardEnabled();
    const QString failure =
        check ? validateVersions() : QString();

    if (!failure.isEmpty()) {
        QTextStream(stderr) << failure << Qt::endl;

        if (!QCoreApplication::arguments().contains("--version-check"))
            QMessageBox::critical(
                nullptr, "RecallForge version mismatch", failure);

        return 78;
    }

    if (QCoreApplication::arguments().contains("--version-check")) {
        QTextStream(stdout)
            << (check
                ? "RecallForge components match ver.json"
                : "RecallForge local version check disabled in settings.json")
            << Qt::endl;
        return 0;
    }

    MainWindow window;
    window.show();
    return app.exec();
}
