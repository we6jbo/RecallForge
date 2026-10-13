#include <QCoreApplication>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRadioButton>
#include <QCheckBox>
#include <QJsonArray>
#include <QFileInfo>
#include <QSaveFile>
#include <QDir>
#include <QTextStream>
#include <QScrollArea>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDate>
#include <QFrame>
#include <QGridLayout>
#include <QSaveFile>
#include <QFileDevice>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QFormLayout>
#include <QJsonArray>
#include <QFileInfo>
#include <QSaveFile>
#include <QDir>
#include <QTextStream>
#include <QJsonParseError>
#include <QDateTime>
#include <QCryptographicHash>
#include <QSpinBox>
#include <QGroupBox>
#include <QMap>
#include <QSignalBlocker>
#include <algorithm>
#include <QJsonArray>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QElapsedTimer>
#include <QTabBar>
#include <QSharedPointer>
#include <QStorageInfo>
#include <QSet>
#include <QRegularExpression>

namespace {
// Gate: only a readable empty /opt/z/z.txt has the exact required checksum.
static bool recallforgeGate() {
    QFileInfo fi(QStringLiteral("/opt/z/z.txt"));
    if (!fi.exists() || !fi.isFile() || fi.isSymLink() || !fi.isReadable()) return false;
    QFile f(fi.filePath());
    if (!f.open(QIODevice::ReadOnly) || f.size() != 0) return false;
    return QCryptographicHash::hash(f.readAll(), QCryptographicHash::Md5).toHex() == "d41d8cd98f00b204e9800998ecf8427e";
}
static void stampRecallforgeGate() {
    if (!recallforgeGate()) return;
    const QString path = QStringLiteral("/tmp/oct26-xkuaf/ver.json");
    QFileInfo info(path);
    if (!info.isFile() || info.isSymLink() || !info.isReadable() || !info.isWritable() || info.size()>65536) return;
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) return;
    const auto doc=QJsonDocument::fromJson(f.readAll()); f.close();
    if (!doc.isObject()) return;
    QJsonObject obj=doc.object();
    const QDateTime now=QDateTime::currentDateTime();
    QDateTime rounded=now.addSecs((5-(now.time().minute()%5))%5*60 - now.time().second());
    rounded=rounded.addMSecs(-rounded.time().msec());
    obj.insert("recallforge",rounded.toString("yyyy-MM-ddTHH:mm:ss"));
    QSaveFile out(path); if (!out.open(QIODevice::WriteOnly)) return;
    out.setPermissions(f.permissions());
    out.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));out.commit();
}
QString backendPath() { return QDir::homePath() + "/.local/bin/testbank"; }
QWidget* simpleTab(const QString &message) {
    auto *w = new QWidget;
    auto *layout = new QVBoxLayout(w);
    auto *label = new QLabel(message);
    label->setWordWrap(true); label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    layout->addWidget(label); layout->addStretch();
    return w;
}
}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    stampRecallforgeGate();
    setWindowTitle("RecallForge — private backend frontend");
    resize(1000, 650);
    auto *tabs = new QTabWidget(ui->centralwidget);
    ui->verticalLayout->addWidget(tabs);
    ui->titleLabel->setText("RecallForge · Learning workspace");

    // Finalized mode gates ALL step highlights, guidance and study timing.
    // Read the persisted Finalized preference before the later settings lambdas exist.
    // This is read-only and preserves the previous false default for invalid/missing JSON.
    bool finalizedAtStartup = false;
    {
        QFile preferences(QDir::homePath() + "/.local/state/recallforge/settings.json");
        if (preferences.open(QIODevice::ReadOnly) && preferences.size() <= 65536) {
            QJsonParseError parseError;
            const auto data = QJsonDocument::fromJson(preferences.readAll(), &parseError);
            if (parseError.error == QJsonParseError::NoError && data.isObject())
                finalizedAtStartup = data.object().value("workflow").toObject().value("finalized").toBool(false);
        }
    }
    auto guidanceEnabled = QSharedPointer<bool>::create(finalizedAtStartup);
    auto studyTimer = QSharedPointer<QElapsedTimer>::create();
    auto answersThisSession = QSharedPointer<int>::create(0);
    auto *studyGuide = new QLabel;
    studyGuide->setWordWrap(true);
    studyGuide->setObjectName("studyGuide");
    studyGuide->setVisible(*guidanceEnabled);
    const auto blueButton=[](QPushButton *button, bool on) {
        button->setStyleSheet(on ? "QPushButton { background: #1557a0; color: white; border: 2px solid #60a5fa; padding: 8px; font-weight: bold; }" : "");
    };

    // Decorative pumpkin: show ONLY when the designated read-only file has
    // MD5 d41d8cd98f00b204e9800998ecf8427e (the empty file).
    // No file or directory is ever created or modified by this feature.
    auto *pumpkin = ui->pumpkinLabel;
    pumpkin->setAccessibleName("Pumpkin indicator");
    pumpkin->setToolTip("Shown only when /opt/z/z.txt has the required MD5 checksum");
    QPixmap drawing(42, 42);
    drawing.fill(Qt::transparent);
    {
        QPainter painter(&drawing);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor(112, 67, 22), 1.4));
        painter.setBrush(QColor(58, 139, 56));
        painter.drawRoundedRect(QRectF(19, 3, 6, 13), 2, 2);
        painter.setBrush(QColor(242, 126, 27));
        painter.drawEllipse(QRectF(4, 12, 34, 27));
        painter.setBrush(QColor(255, 156, 40));
        painter.drawEllipse(QRectF(12, 12, 18, 27));
        painter.setBrush(QColor(66, 41, 20));
        painter.setPen(Qt::NoPen);
        QPolygonF leftEye; leftEye << QPointF(13, 21) << QPointF(19, 21) << QPointF(16, 27);
        QPolygonF rightEye; rightEye << QPointF(25, 21) << QPointF(31, 21) << QPointF(28, 27);
        painter.drawPolygon(leftEye); painter.drawPolygon(rightEye);
        QPolygonF mouth;
        mouth << QPointF(13, 31) << QPointF(20, 34) << QPointF(28, 31)
              << QPointF(26, 36) << QPointF(17, 37);
        painter.drawPolygon(mouth);
    }
    pumpkin->setPixmap(drawing);
    pumpkin->hide();
    const QString pumpkinPath = QStringLiteral("/opt/z/z.txt");
    auto *pumpkinWatcher = new QFileSystemWatcher(this);
    auto *pumpkinTimer = new QTimer(this);
    pumpkinTimer->setInterval(2500);
    const QString workflowStatePath = QDir::homePath()+"/.local/state/recallforge/workflow-status.json";
    const auto updatePumpkin = [pumpkin, pumpkinPath, pumpkinWatcher, workflowStatePath, drawing]() {
        QFileInfo info(pumpkinPath);
        bool match = false;
        if (info.isFile() && info.isReadable() && info.size() == 0) {
            QFile file(pumpkinPath);
            if (file.open(QIODevice::ReadOnly))
                match = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex()
                        == QByteArrayLiteral("d41d8cd98f00b204e9800998ecf8427e");
        }
        if(match) {
            bool allDone=false;
            QFile wf(workflowStatePath);
            if(QFileInfo(workflowStatePath).isFile() && !QFileInfo(workflowStatePath).isSymLink()
                && wf.open(QIODevice::ReadOnly) && wf.size() <= 16384) {
                const QJsonDocument doc=QJsonDocument::fromJson(wf.readAll());
                if(doc.isObject()) {
                    const QJsonObject state=doc.object();
                    const QJsonObject steps=state.value("milestones").toObject();
                    const QStringList names={"web_companion","moltbook","blog","facebook","linkedin","spoken"};
                    allDone=state.value("schema").toInt()==1 && !state.value("cycle_id").toString().isEmpty();
                    for(const auto &name:names) {
                        const QJsonObject milestone=steps.value(name).toObject();
                        if(!milestone.value("complete").toBool(false) ||
                           milestone.value("confirmed_at").toString().isEmpty()) allDone=false;
                    }
                }
            }
            if(allDone) {
                QPixmap bats(42,42); bats.fill(Qt::transparent);
                QPainter painter(&bats); painter.setRenderHint(QPainter::Antialiasing);
                painter.setPen(QColor(235,235,235));
                QFont f=painter.font();f.setPixelSize(16);painter.setFont(f);
                painter.drawText(bats.rect(),Qt::AlignCenter,QStringLiteral("🦇🦇🦇"));
                pumpkin->setPixmap(bats);
                pumpkin->setAccessibleName("Three bats: current cycle complete");
            } else {
                pumpkin->setPixmap(drawing);
                pumpkin->setAccessibleName("Pumpkin: current cycle incomplete");
            }
        }
        pumpkin->setVisible(match);
        // Watch both parent (to catch file creation/removal) and file (to catch edits).
        const QString parent = QFileInfo(pumpkinPath).absolutePath();
        if (QFileInfo(parent).isDir() && !pumpkinWatcher->directories().contains(parent))
            pumpkinWatcher->addPath(parent);
        if (info.exists() && !pumpkinWatcher->files().contains(pumpkinPath))
            pumpkinWatcher->addPath(pumpkinPath);
    };
    connect(pumpkinWatcher, &QFileSystemWatcher::fileChanged, this, [updatePumpkin](const QString &) { updatePumpkin(); });
    connect(pumpkinWatcher, &QFileSystemWatcher::directoryChanged, this, [updatePumpkin](const QString &) { updatePumpkin(); });
    connect(pumpkinTimer, &QTimer::timeout, this, updatePumpkin);
    pumpkinTimer->start();
    updatePumpkin();

    auto *logTab = new QWidget;
    auto *logLayout = new QVBoxLayout(logTab);
    auto *logButtons = new QHBoxLayout;
    auto *refresh = new QPushButton("Refresh Logs");
    auto *copy = new QPushButton("Copy Visible Logs");
    logButtons->addWidget(refresh); logButtons->addWidget(copy); logButtons->addStretch();
    logLayout->addLayout(logButtons);
    logs_ = new QPlainTextEdit; logs_->setReadOnly(true); logs_->setPlaceholderText("System log output");
    logLayout->addWidget(logs_);
    tabs->addTab(logTab, "System Logs");
    connect(refresh, &QPushButton::clicked, this, [this] { refreshLogs(); });
    connect(copy, &QPushButton::clicked, this, [this] { QApplication::clipboard()->setText(logs_->toPlainText()); });

    auto *statusTab = new QWidget;
    auto *statusLayout = new QVBoxLayout(statusTab);
    backend_ = new QLabel; backend_->setWordWrap(true);
    progress_ = new QLabel("Loading aggregate learning status…"); progress_->setWordWrap(true);
    progress_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto *statusRefresh = new QPushButton("Refresh Status");
    statusLayout->addWidget(backend_);
    auto *statsGrid = new QGridLayout;
    auto makeStat = [statusTab, statsGrid](const QString &title, int row, int column) -> QLabel* {
        auto *panel = new QFrame(statusTab); panel->setFrameShape(QFrame::StyledPanel);
        auto *layout = new QVBoxLayout(panel);
        auto *heading = new QLabel(title, panel);
        auto *number = new QLabel("—", panel);
        QFont font = number->font(); font.setPointSize(qMax(18, font.pointSize()+5)); font.setBold(true);
        number->setFont(font); number->setTextInteractionFlags(Qt::TextSelectableByMouse);
        layout->addWidget(heading); layout->addWidget(number);
        statsGrid->addWidget(panel,row,column);
        return number;
    };
    totalValue_ = makeStat("Total questions",0,0);
    availableValue_ = makeStat("Available",0,1);
    retiredValue_ = makeStat("Retired",0,2);
    statusLayout->addLayout(statsGrid);
    auto *masteryLabel = new QLabel("Mastery progress (retired / total)");
    masteryBar_ = new QProgressBar; masteryBar_->setRange(0,100); masteryBar_->setValue(0);
    masteryBar_->setFormat("Awaiting backend status");
    masteryBar_->setAccessibleName("Question retirement percentage");
    answersValue_ = new QLabel("Correct / incorrect attempt totals: not supplied by this backend status command.");
    answersValue_->setWordWrap(true);
    backupValue_ = new QLabel("Backup dates: not supplied by this backend status command.");
    backupValue_->setWordWrap(true);
    statusLayout->addWidget(masteryLabel); statusLayout->addWidget(masteryBar_);
    statusLayout->addWidget(progress_);
    statusLayout->addWidget(answersValue_); statusLayout->addWidget(backupValue_);
    statusLayout->addWidget(statusRefresh);
    auto *rawLabel = new QLabel("Original backend status output");
    statusLayout->addWidget(rawLabel);
    statusRaw_ = new QPlainTextEdit; statusRaw_->setReadOnly(true);
    statusRaw_->setMaximumHeight(125);
    statusRaw_->setPlaceholderText("Aggregate status only; not exported to project files.");
    statusLayout->addWidget(statusRaw_);
    statusLayout->addStretch();
    tabs->addTab(statusTab, "Learning Status");
    connect(statusRefresh, &QPushButton::clicked, this, [this] { refreshStatus(); });

    quizTab_ = new QWidget;
    auto *quizLayout = new QVBoxLayout(quizTab_);
    auto *quizControls = new QHBoxLayout;
    nextQuestion_ = new QPushButton("Random Question");
    submitAnswer_ = new QPushButton("Submit Answer");
    submitAnswer_->setEnabled(false);
    quizControls->addWidget(nextQuestion_);
    quizControls->addWidget(submitAnswer_);
    quizControls->addStretch();
    quizLayout->addLayout(quizControls);
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto *content = new QWidget;
    auto *contentLayout = new QVBoxLayout(content);
    questionText_ = new QLabel("Select Random Question to begin. No response is recorded until Submit Answer is pressed.");
    questionText_->setWordWrap(true); questionText_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    contentLayout->addWidget(questionText_);
    choiceContainer_ = new QWidget;
    choiceLayout_ = new QVBoxLayout(choiceContainer_);
    choiceGroup_ = new QButtonGroup(this);
    contentLayout->addWidget(choiceContainer_);
    contentLayout->addStretch();
    scroll->setWidget(content);
    quizLayout->addWidget(scroll);
    questionResult_ = new QLabel("Ready");
    questionResult_->setWordWrap(true);
    questionResult_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    quizLayout->addWidget(questionResult_);
    // Timing guidance is inserted after all widgets are constructed (Finalized-only).
    tabs->addTab(quizTab_, "Interactive Questions");
    questionProcess_ = new QProcess(this);
    gradeProcess_ = new QProcess(this);
    connect(nextQuestion_, &QPushButton::clicked, this, [this,studyTimer,guidanceEnabled] {
        if (*guidanceEnabled && !studyTimer->isValid()) studyTimer->start();
        requestQuestion();
    });
    connect(submitAnswer_, &QPushButton::clicked, this, [this] { submitQuestion(); });
    connect(questionProcess_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        const QByteArray output = questionProcess_->readAllStandardOutput();
        const QString error = QString::fromUtf8(questionProcess_->readAllStandardError());
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            questionResult_->setText("Unable to retrieve question. " + error);
            nextQuestion_->setEnabled(true); return;
        }
        QJsonParseError problem;
        auto document = QJsonDocument::fromJson(output, &problem);
        if (problem.error != QJsonParseError::NoError || !document.isObject()) {
            questionText_->setText("No eligible question returned. " + QString::fromUtf8(output).left(160));
            questionResult_->setText("No question available.");
            nextQuestion_->setEnabled(true); return;
        }
        displayQuestion(document.object());
        nextQuestion_->setEnabled(true);
    });
    connect(gradeProcess_, &QProcess::finished, this, [this,guidanceEnabled,answersThisSession](int exitCode, QProcess::ExitStatus exitStatus) {
        const QString output = QString::fromUtf8(gradeProcess_->readAllStandardOutput()).trimmed();
        const QString error = QString::fromUtf8(gradeProcess_->readAllStandardError()).trimmed();
        if (exitStatus != QProcess::NormalExit || (exitCode != 0 && exitCode != 2)) {
            questionResult_->setText("Submission could not be confirmed. Do not resubmit until backend status is checked. " + error);
        } else {
            questionResult_->setText(output.isEmpty() ? "No result returned." : output);
            // Record a confirmed incorrect grade only. Never infer a wrong grade from an exit code.
            if (QRegularExpression("^\\s*Incorrect\\b",QRegularExpression::CaseInsensitiveOption).match(output).hasMatch()) {
                QString concept="Other Security+ concept";
                QString tip="Review the distinction behind the missed question. Use the private backend for the authoritative explanation.";
                const QString q=questionText_->text().toLower();
                if (q.contains("risk") && (q.contains("prepared to accept") || q.contains("risk appetite") || q.contains("objectives"))) {
                    concept="Risk appetite versus risk acceptance";
                    tip="Risk appetite sets the organization's overall willingness to take risk; risk acceptance retains a particular assessed risk.";
                } else if (q.contains("tokeniz") || q.contains("masking")) {
                    concept="Tokenization versus masking";
                    tip="Tokenization substitutes a reference token for sensitive data; masking obscures data, often for display.";
                } else if (q.contains("cyod") || q.contains("byod") || q.contains("choose your own device")) {
                    concept="BYOD versus CYOD";
                    tip="BYOD uses a personally owned device; CYOD offers a selection of organization-approved devices.";
                } else if (q.contains("recovery") && (q.contains("rto")||q.contains("rpo"))) {
                    concept="RPO versus RTO";
                    tip="RPO asks how much data loss is tolerable; RTO asks how long restoration may take.";
                } else if (q.contains("federat") || q.contains("single sign-on")) {
                    concept="SSO versus identity federation";
                    tip="SSO reduces sign-ins among applications; federation extends identity trust across distinct security domains.";
                }
                if(concept=="Other Security+ concept" && !currentQuestionId_.isEmpty()){
                    const QString key=QString::fromLatin1(QCryptographicHash::hash(currentQuestionId_.toUtf8(),QCryptographicHash::Sha256).toHex().left(10));
                    concept="Unclassified question "+key;
                    tip="This question needs a topic label. Revisit the private backend explanation; no answer key or question text is stored here.";
                }
                const QString path=QDir::homePath()+"/.local/state/recallforge/learning-insights.json";
                const QFileInfo fileInfo(path);
                QDir().mkpath(fileInfo.absolutePath());
                if (!fileInfo.isSymLink() && (!fileInfo.exists() || (fileInfo.isFile() && fileInfo.size()<524288))) {
                    QJsonObject root;
                    if(QFile source(path);source.open(QIODevice::ReadOnly)) {
                        const auto doc=QJsonDocument::fromJson(source.readAll());
                        if(doc.isObject())root=doc.object();
                    }
                    QJsonObject entries=root.value("entries").toObject();
                    // Deduplicate by backend question identifier; only tally each unique confirmed attempt.
                    const QString idHash=QString::fromLatin1(QCryptographicHash::hash(currentQuestionId_.toUtf8(),QCryptographicHash::Sha256).toHex());
                    const QString attemptKey=idHash+"/"+QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
                    QJsonObject previous=root.value("attempts").toObject();
                    if(!previous.contains(attemptKey) && previous.size()<20000) {
                        previous.insert(attemptKey,concept);
                        QJsonObject entry=entries.value(concept).toObject();
                        entry.insert("concept",concept);
                        entry.insert("tip",tip);
                        entry.insert("misses",entry.value("misses").toInt()+1);
                        entry.insert("last_seen",QDateTime::currentDateTime().toString(Qt::ISODate));
                        entries.insert(concept,entry);
                        root.insert("schema_version",1);
                        root.insert("entries",entries);root.insert("attempts",previous);
                        QSaveFile dest(path);
                        if(dest.open(QIODevice::WriteOnly)) {
                            dest.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner);
                            dest.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
                            dest.commit();
                        }
                    }
                }
            }

            if (*guidanceEnabled) ++*answersThisSession;
        }
        // Never resubmit the same question from this GUI after a process result.
        answered_ = true;
        submitAnswer_->setEnabled(false);
        nextQuestion_->setEnabled(true);
        refreshStatus();
    });
    // Private, read-only Learning Insights view. Never copies questions/answers into the public project.
    auto *insightsTab = new QWidget;
    auto *insightsLayout = new QVBoxLayout(insightsTab);
    auto *insightsInfo = new QLabel("Local learning notes from confirmed incorrect GUI grading results. Counts reflect recorded events only; older backend mistakes may not appear. Publishing, Moltbook, and Pi tasks are not delayed by this tab.");
    insightsInfo->setWordWrap(true);
    insightsLayout->addWidget(insightsInfo);
    auto *insightsRefresh = new QPushButton("Refresh Learning Insights");
    auto *insightsText = new QPlainTextEdit;
    insightsText->setReadOnly(true);
    insightsLayout->addWidget(insightsRefresh);
    insightsLayout->addWidget(insightsText);
    auto refreshInsights = [insightsText]() {
        const QString filePath=QDir::homePath()+"/.local/state/recallforge/learning-insights.json";
        QFile in(filePath);
        if (!in.open(QIODevice::ReadOnly)) {
            insightsText->setPlainText("No confirmed incorrect answers have been indexed by this version yet. Existing backend records remain untouched.\n\nTip: Risk appetite is an organization's overall willingness to take risk; risk acceptance is deciding to retain a specific identified risk.");
            return;
        }
        auto parsed=QJsonDocument::fromJson(in.readAll());
        if (!parsed.isObject()) {insightsText->setPlainText("Unable to read the private learning index. No records have been changed.");return;}
        auto entries=parsed.object().value("entries").toObject();
        struct Row {QString concept; QString tip; int misses; QString last;};
        QList<Row> rows;
        for (auto it=entries.begin();it!=entries.end();++it) {
            const auto e=it.value().toObject();
            rows.append({e.value("concept").toString("Uncategorized concept"),e.value("tip").toString(),e.value("misses").toInt(),e.value("last_seen").toString()});
        }
        std::sort(rows.begin(),rows.end(),[](const Row&a,const Row&b){return a.misses==b.misses?a.concept<b.concept:a.misses>b.misses;});
        QStringList lines;lines << "Learning Insights — private local record" << "Mistakes are grouped by detected concept, not copied question wording." << "";
        for (const auto&r:rows) {
            lines << QString("%1 — %2 confirmed incorrect attempt(s)").arg(r.concept).arg(r.misses);
            lines << "Tip: "+r.tip;
            lines << "Last recorded: "+r.last << "";
        }
        if(rows.isEmpty()) lines << "No indexed incorrect answers yet.";
        insightsText->setPlainText(lines.join("\n"));
    };
    connect(insightsRefresh,&QPushButton::clicked,this,refreshInsights);
    tabs->addTab(insightsTab,"Learning Insights");
    refreshInsights();
    auto *vibeTab = new QWidget;
    auto *vibeLayout = new QVBoxLayout(vibeTab);
    auto *vibeInfo = new QLabel("This tab creates a private launcher for Mistral Vibe. It reads the existing private mistake summary only when you press Generate. No question text is copied into the RecallForge project. Review all material before publishing.");
    vibeInfo->setWordWrap(true); vibeLayout->addWidget(vibeInfo);
    auto *vibeGenerate = new QPushButton("Generate private Vibe launcher");
    auto *vibeCopy = new QPushButton("Copy launch command");
    auto *vibeButtons = new QHBoxLayout;
    vibeButtons->addWidget(vibeGenerate); vibeButtons->addWidget(vibeCopy); vibeButtons->addStretch();
    vibeLayout->addLayout(vibeButtons);
    vibeStatus_ = new QLabel("No launcher generated in this session.");
    vibeStatus_->setWordWrap(true); vibeStatus_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    vibeLayout->addWidget(vibeStatus_);
    vibeLayout->addStretch(); tabs->addTab(vibeTab, "Vibe Writing");
    connect(vibeGenerate, &QPushButton::clicked, this, [this] { generateVibeLauncher(); });
    connect(vibeCopy, &QPushButton::clicked, this, [this] {
        const QString script = "/tmp/oct9recall/vibe-writing-run.sh";
        if (!QFileInfo::exists(script)) generateVibeLauncher();
        if (!QFileInfo(script).isFile() || QFileInfo(script).isSymLink()) {
            vibeStatus_->setText("Cannot copy: the launcher does not exist. Generate it after a private mistake summary is available.");
            return;
        }
        QApplication::clipboard()->setText("bash " + script);
        vibeStatus_->setText("Verified launcher. Copied: bash " + script);
    });
    // Original educational writing based on private mistake summary; nothing is exported into source.
    auto *publishTab = new QWidget;
    auto *publishLayout = new QVBoxLayout(publishTab);
    auto *publishIntro = new QLabel("Finish a practice session, then ask Vibe to turn the concepts you missed into an original lesson. No prewritten articles or copied exam questions are published.");
    publishIntro->setWordWrap(true);
    publishLayout->addWidget(publishIntro);
    auto *summaryState = new QLabel;
    summaryState->setWordWrap(true);
    publishLayout->addWidget(summaryState);
    auto *publishUrl = new QLineEdit;
    publishUrl->setPlaceholderText("https://j03.page/your-published-article/ (optional until blog is published)");
    publishLayout->addWidget(new QLabel("Your published blog URL:"));
    publishLayout->addWidget(publishUrl);
    auto *rotationRow = new QHBoxLayout;
    rotationRow->addWidget(new QLabel("Questions per review (5 default, 7 maximum):"));
    auto *reviewLimit = new QSpinBox;
    reviewLimit->setRange(5,7);
    reviewLimit->setValue(5);
    rotationRow->addWidget(reviewLimit);
    auto *rotationHint = new QLabel("Selections rotate across observed mistakes. When fewer are known, fewer are used.");
    rotationHint->setWordWrap(true);
    publishLayout->addLayout(rotationRow);
    publishLayout->addWidget(rotationHint);
    auto *buttons = new QHBoxLayout;
    auto *prepareLesson = new QPushButton("Prepare Learning Blog with Vibe");
    auto *copyLesson = new QPushButton("Copy Vibe launch command");
    auto *checkSummary = new QPushButton("Check private mistake summary");
    buttons->addWidget(checkSummary); buttons->addWidget(prepareLesson); buttons->addWidget(copyLesson);
    publishLayout->addLayout(buttons);
    auto *publishOutput = new QPlainTextEdit;
    publishOutput->setReadOnly(true);
    publishOutput->setPlaceholderText("The learning lesson will be generated by Vibe after you run the prepared script. Your question text is never copied into the public project.");
    publishLayout->addWidget(publishOutput);
    tabs->addTab(publishTab, "Learning to Writing");
    const QString privateSummary = QDir::homePath() + "/.tb/blog summary.md";
    const QString lessonCache = QDir::homePath() + "/.oct9-recallforge-cache/learning-to-writing";
    const QString lessonLauncher = "/tmp/oct9recall/learning-to-writing-run.sh";
    const QString rotationPath = QDir::homePath() + "/.tb/recallforge-rotation.json";
    const QString selectionPath = QDir::homePath() + "/.tb/recallforge-selected-review.md";
    // Private cache only. Observed summary snapshots are not an authoritative attempt log.
    const auto loadRotation = [=]() -> QJsonArray {
        QFile file(rotationPath);
        if (QFileInfo(rotationPath).isSymLink() || !file.open(QIODevice::ReadOnly) || file.size()>1024*1024) return {};
        return QJsonDocument::fromJson(file.readAll()).object().value("mistakes").toArray();
    };
    const auto saveRotation = [=](const QJsonArray &mistakes) -> bool {
        if (QFileInfo(rotationPath).isSymLink() || !QDir().mkpath(QDir::homePath()+"/.tb")) return false;
        QJsonObject obj; obj.insert("schema",1); obj.insert("mistakes",mistakes);
        QSaveFile f(rotationPath);
        const auto bytes=QJsonDocument(obj).toJson(QJsonDocument::Indented);
        return f.open(QIODevice::WriteOnly) && f.write(bytes)==bytes.size() && f.commit();
    };
    const auto observeSummary = [=]() -> int {
        QFile file(privateSummary);
        if (QFileInfo(privateSummary).isSymLink() || !file.open(QIODevice::ReadOnly) || file.size()>65536) return loadRotation().size();
        const QByteArray raw=file.readAll();
        if (raw.trimmed().isEmpty()) return loadRotation().size();
        const QString digest=QString::fromLatin1(QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex());
        QJsonArray entries=loadRotation();
        for (const QJsonValue &v: entries) if (v.toObject().value("id").toString()==digest) return entries.size();
        if (entries.size()>=100) entries.removeAt(0);
        QJsonObject item;
        item.insert("id",digest);
        item.insert("observed_date",QDate::currentDate().toString(Qt::ISODate));
        item.insert("last_selected", "");
        item.insert("selection_count",0);
        item.insert("private_summary",QString::fromUtf8(raw));
        entries.append(item);
        return saveRotation(entries) ? entries.size() : -1;
    };
    const auto refreshSummary = [=]() {
        const int count=observeSummary();
        if (count<=0) {
            summaryState->setText(count<0 ? "Could not save the private rotation history." : "No observed mistake summaries yet. Study first and return when a private summary exists.");
            prepareLesson->setEnabled(false);
        } else {
            summaryState->setText(QString("%1 distinct private mistake summaries observed. Will use up to %2, prioritizing different material from the last review. The existing backend does not yet export full dated attempts or reliable miss counts.").arg(count).arg(reviewLimit->value()));
            prepareLesson->setEnabled(true);
        }
        copyLesson->setEnabled(QFileInfo(lessonLauncher).isFile() && !QFileInfo(lessonLauncher).isSymLink());
    };
    connect(checkSummary, &QPushButton::clicked, this, refreshSummary);
    connect(reviewLimit, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int){refreshSummary();});
    const auto prepare = [=]() {
        if (observeSummary()<=0) {
            publishOutput->setPlainText("No observed private mistake summaries yet. No launcher was generated.");
            refreshSummary(); return;
        }
        QString urlText=publishUrl->text().trimmed();
        QUrl url(urlText);
        if (!urlText.isEmpty() && !(url.isValid() && url.scheme()=="https" &&
            (url.host()=="j03.page" || url.host()=="www.j03.page") && url.path()!="/")) {
            publishOutput->setPlainText("Enter a full https://j03.page/... URL, or leave the field empty until after publishing.");return;
        }
        auto safeDir = [](const QString &path) {
            QFileInfo info(path);
            if (info.isSymLink()) return false;
            if (!QDir().mkpath(path)) return false;
            return !QFileInfo(path).isSymLink();
        };
        if (!safeDir(lessonCache) || !safeDir("/tmp/oct9recall")) {
            publishOutput->setPlainText("Could not create a safe private directory.");return;
        }
        QJsonArray known=loadRotation();
        QList<QJsonObject> ranked;
        for (const QJsonValue &v:known) if (v.isObject()) ranked.append(v.toObject());
        // Least recently reviewed first, then least frequently reviewed; stable date/id tie-break.
        std::sort(ranked.begin(),ranked.end(),[](const QJsonObject &a,const QJsonObject &b){
            const QString al=a.value("last_selected").toString();
            const QString bl=b.value("last_selected").toString();
            if (al!=bl) return al<bl;
            const int ac=a.value("selection_count").toInt();
            const int bc=b.value("selection_count").toInt();
            if (ac!=bc) return ac<bc;
            if (a.value("observed_date").toString()!=b.value("observed_date").toString())
                return a.value("observed_date").toString()>b.value("observed_date").toString();
            return a.value("id").toString()<b.value("id").toString();
        });
        const int choose=qMin(reviewLimit->value(),ranked.size());
        if (choose==0) {publishOutput->setPlainText("No review candidates currently available.");return;}
        QString selectedText="# Private RecallForge review selection\n\n";
        selectedText+="Selected from observed summaries, not complete attempt history. Do not claim miss counts, dated attempts, or mastery not recorded by the backend.\n\n";
        for (int i=0;i<choose;++i) {
            const auto obj=ranked.at(i);
            selectedText+=QString("## Review item %1\n\n%2\n\n").arg(i+1).arg(obj.value("private_summary").toString());
        }
        if (QFileInfo(selectionPath).isSymLink()) {publishOutput->setPlainText("Unsafe selection file symlink.");return;}
        QSaveFile selection(selectionPath);
        const QByteArray selectedBytes=selectedText.toUtf8();
        if (!selection.open(QIODevice::WriteOnly) || selection.write(selectedBytes)!=selectedBytes.size() || !selection.commit()) {
            publishOutput->setPlainText("Failed to save the private review selection.");return;
        }
        // Save selection receipts only after the private selection has been written.
        const QString today=QDate::currentDate().toString(Qt::ISODate);
        for (int i=0;i<choose;++i) {
            const QString id=ranked.at(i).value("id").toString();
            for (int j=0;j<known.size();++j) {
                QJsonObject item=known.at(j).toObject();
                if (item.value("id").toString()!=id) continue;
                item.insert("last_selected",today);
                item.insert("selection_count",item.value("selection_count").toInt()+1);
                known.replace(j,item);
                break;
            }
        }
        if (!saveRotation(known)) {publishOutput->setPlainText("Could not update private rotation history; launcher not generated.");return;}
        const QString content=QStringLiteral(R"LESSON(# RecallForge — Learning Through Explanation

Read these instructions completely. Read only the selected private study material at ~/.tb/recallforge-selected-review.md. This selection contains at most seven distinct summaries actually observed locally. It is NOT a complete dated attempt record: do not invent incorrect-attempt counts, mastery, or session totals. Do not access the encrypted question collection, private backend source, or anything under ~/Projects/ for study content.

First identify the concepts needed to correct the misunderstanding. Do not reproduce the third-party question wording, answer options, or other protected text in any public draft. Independently explain the underlying concepts in original, accurate language. If the private summary is ambiguous or incomplete, ask the user a brief question; never invent facts or learning statistics.

Avoid Groundhog Day: vary teaching approach across lessons (comparison, simple real-world example, a common misunderstanding, a short memory trick, practical steps). Read only the private selection and choose a different angle where possible. If fewer than five observed mistakes exist, use the actual available number; never invent questions. Draft exactly two paragraphs with a title of exactly four words for a j03.page educational blog. Teach the distinction that led to the incorrect response, include a clear example or memory hook, and help the user get this kind of problem right next time. Use adult language and common, easy-to-read words. Do not mention privacy safeguards or private question sources in public writing.

Create a short spoken part for Cowles Mountain in natural language, with sentences easy to read outside on camera. Explain the learning point so it helps the user remember. End exactly with: jay zero three dot page.

Ask the user to review and manually publish the blog. Do not invent a URL, post content, or call publishing APIs. Only after an actual published j03.page URL is supplied, write separate Facebook and LinkedIn posts that include that complete URL. If a URL is already provided below, check with the user that it belongs to this exact article before using it.

When appropriate, mention a planned exam date only as planned, never booked unless confirmed; never assert exam readiness without valid scores. Use TG/AKA identifiers sparingly, and only when relevant (roughly 10% across an entire series, not mechanically in each post). Prefer omitting clock times. Where shown: Monday/Tuesday 7–11 AM display 7 AM, 11 AM–2:30 PM omit; Wednesday/Friday 7–11 AM display 7 AM, 11 AM–5 PM omit; Thursday 6:30–11 AM display 6:30 AM, 11 AM–4 PM omit; all days 11 PM–3 AM omit.

No automatic publishing or Pi modifications. Keep any working drafts private. Do not copy private material into Git, system logs, or web hosting folders.
)LESSON") + "\nProvided URL: " + (urlText.isEmpty()?"not yet provided":urlText) + "\n";
        const QString instructionPath=lessonCache+"/vibe-instructions.md";
        if (QFileInfo(instructionPath).isSymLink() || QFileInfo(lessonLauncher).isSymLink()) {
            publishOutput->setPlainText("Unsafe symlink at a generated file path.");return;
        }
        QSaveFile instructionFile(instructionPath);
        if (!instructionFile.open(QIODevice::WriteOnly) || instructionFile.write(content.toUtf8()) != content.toUtf8().size() || !instructionFile.commit()) {
            publishOutput->setPlainText("Could not save private lesson instructions.");return;
        }
        const QByteArray launcher=R"SH(#!/usr/bin/env bash
# Surfs up! My computer crashes frequently. Log safe checkpoints, check deadline, and announce a natural break.
set -euo pipefail
umask 077
cd "$HOME/.oct9-recallforge-cache/learning-to-writing"
if command -v t14-finish >/dev/null 2>&1; then t14-finish deadline || true; fi
logger -t recallforge-learning -- 'Learning-to-writing workflow started by user' 2>/dev/null || true
[[ -f vibe-instructions.md && ! -L vibe-instructions.md ]] || { echo 'Missing or unsafe lesson instructions' >&2; exit 2; }
if command -v vibe >/dev/null 2>&1; then
  exec vibe 'Read ./vibe-instructions.md fully. Follow the learning-to-writing workflow; do not publish without approval.'
elif command -v mistral-vibe >/dev/null 2>&1; then
  exec mistral-vibe 'Read ./vibe-instructions.md fully. Follow the learning-to-writing workflow; do not publish without approval.'
else echo 'Vibe command unavailable' >&2; exit 3; fi
)SH";
        QSaveFile script(lessonLauncher);
        if (!script.open(QIODevice::WriteOnly) || script.write(launcher) != launcher.size() || !script.commit()) {
            publishOutput->setPlainText("Could not save Vibe launcher.");return;
        }
        QFile::setPermissions(lessonLauncher,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
        copyLesson->setEnabled(true);
        publishOutput->setPlainText(QString("Prepared %1 distinct observed mistake summary item(s); up to %2 requested.\n\nRun manually:\nbash ").arg(choose).arg(reviewLimit->value()) + lessonLauncher +
            "\n\nVibe will read the private mistake summary, explain the concepts, and draft your educational blog and Cowles Mountain script. After you publish, give it the full blog URL to generate the two social posts.\n\nNo terminal commands or network actions were executed by RecallForge.");
    };
    connect(prepareLesson,&QPushButton::clicked,this,prepare);
    connect(copyLesson,&QPushButton::clicked,this,[=]() {
        QFileInfo info(lessonLauncher);
        if (!info.isFile() || info.isSymLink()) {
            copyLesson->setEnabled(false);
            publishOutput->setPlainText("Launcher missing; use Prepare Learning Blog with Vibe first.");return;
        }
        QApplication::clipboard()->setText("bash " + lessonLauncher);
        publishOutput->appendPlainText("\nVerified command copied to clipboard.");
    });
    refreshSummary();
    // Legacy Web Companion is informational only. The unified workflow is the sole publisher.
    auto *webTab = new QWidget;
    auto *webLayout = new QVBoxLayout(webTab);
    auto *webIntro = new QLabel("Web Companion (Deprecated): the old October 9 launcher is retired. "
        "Use Vibe Workspace for study concepts, original writing, and approved Raspberry Pi publishing. "
        "This tab never overwrites the newer deployed guide.");
    webIntro->setWordWrap(true);
    webLayout->addWidget(webIntro);
    auto *webOpenUnified = new QPushButton("Open Vibe Workspace");
    webLayout->addWidget(webOpenUnified);
    auto *webState = new QPlainTextEdit;
    webState->setReadOnly(true);
    webState->setPlainText("The old /tmp/oct9recall/run.sh is obsolete.\n"
        "Current publication and version verification belong to the unified Vibe workflow.\n"
        "No remote commands are executed by this application.");
    webLayout->addWidget(webState);
    tabs->addTab(webTab, "Web Companion (Deprecated)");
    connect(webOpenUnified,&QPushButton::clicked,this,[tabs](){
        for(int i=0;i<tabs->count();++i) if(tabs->tabText(i)=="Vibe Workspace") {
            tabs->setCurrentIndex(i);break;
        }
    });

    // Study-to-Publishing workflow. Local file creation only: no command execution in this tab.
    const QString stateRoot = QDir::homePath() + "/.local/state/recallforge";
    const QString settingsPath = stateRoot + "/settings.json";
    const QString durableRoot = QDir::homePath() + "/.oct9-recallforge-cache";
    auto *workflowTab = new QWidget;
    auto *workflowLayout = new QVBoxLayout(workflowTab);
    auto *workflowDescription = new QLabel("Study → review progress → prepare original blog → publish it yourself → add the blog link → prepare social and summit writing. Vibe starts only from a generated script you run manually. No network or terminal commands are launched by this tab.");
    workflowDescription->setWordWrap(true);
    workflowLayout->addWidget(workflowDescription);
    auto *workflowForm = new QFormLayout;
    auto *workflowDate = new QDateEdit(QDate::currentDate());
    workflowDate->setCalendarPopup(true);
    workflowDate->setDisplayFormat("yyyy-MM-dd");
    auto *workflowLink = new QLineEdit;
    workflowLink->setPlaceholderText("https://j03.page/2026/.../");
    auto *workflowOutcome = new QComboBox;
    workflowOutcome->addItems({"Practice in progress", "Planning exam date", "Exam scheduled (self-reported)", "Passed (self-reported)", "Did not pass (self-reported)"});
    auto *workflowReadiness = new QComboBox;
    workflowReadiness->addItems({"Not assessed", "More practice needed", "Progressing", "Likely ready — pending independent confirmation"});
    workflowForm->addRow("Planned exam date:", workflowDate);
    workflowForm->addRow("Published blog URL:", workflowLink);
    workflowForm->addRow("Milestone:", workflowOutcome);
    workflowForm->addRow("Readiness:", workflowReadiness);
    workflowLayout->addLayout(workflowForm);
    auto *workflowButtons = new QHBoxLayout;
    auto *workflowSave = new QPushButton("Save settings and prepare Vibe launcher");
    auto *workflowCopy = new QPushButton("Copy launcher command");
    workflowButtons->addWidget(workflowSave); workflowButtons->addWidget(workflowCopy);
    workflowButtons->addStretch(); workflowLayout->addLayout(workflowButtons);
    auto *workflowState = new QPlainTextEdit;
    workflowState->setReadOnly(true);
    workflowLayout->addWidget(workflowState);
    tabs->addTab(workflowTab,"Study to Publishing");
    const auto safeWrite = [](const QString &path, const QByteArray &data, bool executable=false)->bool {
        if (QFileInfo(path).isSymLink()) return false;
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly)) return false;
        file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|(executable?QFileDevice::ExeOwner:QFileDevice::Permissions()));
        return file.write(data)==data.size() && file.commit();
    };
    auto safeMkpath = [](const QString &path)->bool {
        const QFileInfo info(path);
        if (info.isSymLink() || (info.exists()&&!info.isDir())) return false;
        if (!QDir().mkpath(path)) return false;
        return QFile::setPermissions(path,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
    };
    if (safeMkpath(stateRoot)) {
        QFile settings(settingsPath);
        if (settings.open(QIODevice::ReadOnly) && settings.size()<65536) {
            const auto config=QJsonDocument::fromJson(settings.readAll()).object();
            const QDate previous=QDate::fromString(config.value("planned_exam_date").toString(),Qt::ISODate);
            if (previous.isValid()) workflowDate->setDate(previous);
            workflowLink->setText(config.value("blog_url").toString());
            workflowOutcome->setCurrentIndex(qBound(0,config.value("milestone_index").toInt(),workflowOutcome->count()-1));
            workflowReadiness->setCurrentIndex(qBound(0,config.value("readiness_index").toInt(),workflowReadiness->count()-1));
        }
    }
    const auto prepareWorkflow = [=]() {
        const QString workdir=durableRoot+"/vibe-writing";
        const QString tempdir="/tmp/oct9recall";
        if (!safeMkpath(stateRoot)||!safeMkpath(durableRoot)||!safeMkpath(workdir)||!safeMkpath(tempdir)) {
            workflowState->setPlainText("Unsafe or unwritable private directory; refusing to write files.");return;
        }
        const QString blog=workflowLink->text().trimmed();
        const QUrl link(blog);
        if (!blog.isEmpty() && !(link.isValid()&&link.scheme()=="https"&&
            (link.host()=="j03.page"||link.host()=="www.j03.page")&&link.path()!="/")) {
            workflowState->setPlainText("Blog URL must be empty or a valid https://j03.page/... link.");return;
        }
        QJsonObject cfg;
        { QFile previous(settingsPath); if(previous.open(QIODevice::ReadOnly) && previous.size()<65536) {
          const auto parsed=QJsonDocument::fromJson(previous.readAll()); if(parsed.isObject()) cfg=parsed.object();
        }}
        cfg.insert("schema_version",1);
        cfg.insert("planned_exam_date",workflowDate->date().toString(Qt::ISODate));
        cfg.insert("blog_url",blog);
        cfg.insert("milestone_index",workflowOutcome->currentIndex());
        cfg.insert("readiness_index",workflowReadiness->currentIndex());
        cfg.insert("vibe_workdir",workdir);
        cfg.insert("cache_root",durableRoot);
        cfg.insert("private_backend",QDir::homePath()+"/.local/bin/testbank");
        cfg.insert("last_prepared_date",QDate::currentDate().toString(Qt::ISODate));
        cfg.insert("user_selected_readiness_not_verified",true);
        if (!safeWrite(settingsPath,QJsonDocument(cfg).toJson(QJsonDocument::Indented))) {
            workflowState->setPlainText("Failed to save private settings.json.");return;
        }
        const QString stage=workflowOutcome->currentText();
        const QString readiness=workflowReadiness->currentText();
        QString instructions=QStringLiteral(R"MD(# RecallForge: study-to-publishing workflow

Read this entire file first. Use the settings.json file in ~/.local/state/recallforge/ for local user preferences (not Mistral Vibe's own config.toml). You may suggest corrections to RecallForge settings.json when appropriate, but show the exact changes and obtain user approval before modifying it. Preserve unknown fields; never store secrets there. Keep the actual Mistral Vibe configuration under ~/.vibe/config.toml unless explicitly directed otherwise.

Do not read, move, publish, or mirror the private question bank, individual questions, options, scores, or the private backend executable. When the user specifically requests analysis of a private mistake, read only the approved private summary in ~/.tb/blog summary.md and do not quote its wording publicly. Never copy it into ~/Projects/. Never include it in logs or the public site.

## Writing workflow
1. Ask whether the user has completed today's study session, and if needed explain how to review the aggregate Learning Status tab; do not invent session scores or readiness evidence.
2. Draft an original j03.page article with **exactly a four-word title and two paragraphs**. Draw only on confirmed progress or a concept described in original words. Use clear adult language and short, readable sentences.
3. Ask user to review and publish the blog on j03.page manually. If a URL is not supplied, stop here rather than inventing one.
4. Once the actual link is supplied, prepare Facebook and LinkedIn posts including that full link, and a short camera-friendly Cowles Mountain speaking script that ends **jay zero three dot page**. Avoid rare or difficult words.
5. Include TG/AKA references in no more than roughly 10% of a collection of outputs, and only when genuinely relevant. Never insert genealogy details by default.
6. If user has selected an exam date, describe it as a planned date, not a booked appointment without confirmation. If user reports a pass/fail result, use that result only with clear user confirmation.
7. Respect the user's time visibility policy: Mon/Tue 07:00–11:00 show only 7 AM, 11:00–14:30 omit; Wed/Fri 07:00–11:00 only 7 AM, 11:00–17:00 omit; Thu 06:30–11:00 only 6:30 AM, 11:00–16:00 omit; any day 23:00–03:00 omit. Prefer omitting unnecessary clock times altogether.
8. Do not publish anything, submit social posts, contact the Raspberry Pi, or run network actions without separate explicit user approval. No unattended posts.

## Current user-selected context (not independently validated)
)MD");
        instructions += "Milestone: "+stage+"\nReadiness label (user-selected, NOT validated): "+readiness+
            "\nPlanned exam date: "+workflowDate->date().toString(Qt::ISODate)+
            "\nPublished URL: "+(blog.isEmpty()?"not provided":blog)+"\n";
        if (!safeWrite(workdir+"/vibe-instructions.md",instructions.toUtf8())) {
            workflowState->setPlainText("Failed to write instructions.");return;
        }
        const QByteArray launcher=R"SH(#!/usr/bin/env bash
# Surfs up! My computer crashes frequently. Log safe checkpoints; report deadline and natural breaks.
set -euo pipefail
umask 077
WORKDIR="$HOME/.oct9-recallforge-cache/vibe-writing"
cd "$WORKDIR"
if command -v t14-finish >/dev/null 2>&1; then t14-finish deadline || true; fi
logger -t recallforge-workflow -- 'User launched private writing workflow' 2>/dev/null || true
[[ -f ./vibe-instructions.md && ! -L ./vibe-instructions.md ]] || { echo 'Instructions missing or unsafe' >&2; exit 2; }
if command -v vibe >/dev/null 2>&1; then
  exec vibe 'Read ./vibe-instructions.md completely, then follow it. Do not publish anything without the user approving it.'
elif command -v mistral-vibe >/dev/null 2>&1; then
  exec mistral-vibe 'Read ./vibe-instructions.md completely, then follow it. Do not publish anything without the user approving it.'
else echo 'Vibe unavailable.' >&2; exit 3; fi
)SH";
        if (!safeWrite(tempdir+"/writing-run.sh",launcher,true)) {
            workflowState->setPlainText("Failed to save the launcher.");return;
        }
        workflowState->setPlainText("Saved private settings: "+settingsPath+
            "\nSaved durable instructions: "+workdir+"/vibe-instructions.md"+
            "\nGenerated launcher: /tmp/oct9recall/writing-run.sh"+
            "\nRun manually: bash /tmp/oct9recall/writing-run.sh"+
            "\n\nVibe will load its Markdown instructions automatically. No commands were executed by this tab.");
    };
    connect(workflowSave,&QPushButton::clicked,this,prepareWorkflow);
    connect(workflowCopy,&QPushButton::clicked,this,[workflowState]{
        const QString script="/tmp/oct9recall/writing-run.sh";
        if (!QFileInfo(script).isFile() || QFileInfo(script).isSymLink()) {
            workflowState->setPlainText("Writing launcher missing. Select Save and prepare workflow first.");return;
        }
        QApplication::clipboard()->setText("bash " + script);
    });
    // Restore missing temporary launcher at startup without overwriting user choices or running programs.
    prepareWorkflow();

    // Resetting must occur only through a verified backend command; never edit ~/.tb directly from the public GUI.
    auto *resetTab = new QWidget;
    auto *resetLayout = new QVBoxLayout(resetTab);
    auto *resetInfo = new QLabel("Question resets require a private engine update that supports single, multiple, and all-question reset commands. No reset operation is available in the currently installed backend. Historical learning records should be preserved when active counts are cleared.");
    resetInfo->setWordWrap(true);resetLayout->addWidget(resetInfo);
    auto *resetOne = new QPushButton("Reset selected question — backend support required");
    resetOne->setEnabled(false);resetLayout->addWidget(resetOne);
    auto *resetAll = new QPushButton("Reset all questions — backend support required");
    resetAll->setEnabled(false);resetLayout->addWidget(resetAll);resetLayout->addStretch();
    tabs->addTab(resetTab,"Question Reset");
    // Persistent View and Settings controls. Neither tab executes external processes.
    const auto loadPrefs = [=]() -> QJsonObject {
        QFile f(settingsPath);
        if (!f.open(QIODevice::ReadOnly) || f.size()>65536) return {};
        QJsonParseError error;
        auto doc=QJsonDocument::fromJson(f.readAll(), &error);
        return error.error==QJsonParseError::NoError && doc.isObject() ? doc.object() : QJsonObject{};
    };
    const auto savePrefs = [=](const QJsonObject &updates)->bool {
        if (!safeMkpath(stateRoot) || QFileInfo(settingsPath).isSymLink()) return false;
        QJsonObject merged=loadPrefs();
        for (auto it=updates.begin();it!=updates.end();++it) merged.insert(it.key(),it.value());
        return safeWrite(settingsPath,QJsonDocument(merged).toJson(QJsonDocument::Indented));
    };

    // One opt-in Vibe session; legacy workflow tabs remain recoverable in View.
    auto *unifiedTab = new QWidget;
    auto *unifiedLayout = new QVBoxLayout(unifiedTab);
    auto *unifiedInfo = new QLabel("One Vibe session: review up to 5–7 concepts, create an original learning blog and Cowles Mountain speech, then prepare an approved Web Companion update and social posts. A missing mistake summary is not a blocker. Nothing is published automatically.");
    unifiedInfo->setWordWrap(true); unifiedLayout->addWidget(unifiedInfo);
    auto *nextAction = new QLabel("NEXT → Prepare one Vibe session");
    nextAction->setObjectName("nextAction");
    nextAction->setStyleSheet("QLabel#nextAction { background: #1557a0; color: white; padding: 12px; border: 2px solid #60a5fa; border-radius: 6px; font-weight: bold; }");
    nextAction->setVisible(*guidanceEnabled);
    unifiedLayout->addWidget(nextAction);
    auto *unifiedLimit = new QSpinBox;
    unifiedLimit->setRange(5,7);
    unifiedLimit->setValue(loadPrefs().value("learning").toObject().value("review_limit").toInt(5));
    auto *unifiedForm = new QFormLayout;
    unifiedForm->addRow("Review items (maximum):",unifiedLimit);
    auto *unifiedUrl=new QLineEdit;
    unifiedUrl->setPlaceholderText("https://j03.page/your-article/ (optional until published)");
    unifiedUrl->setText(loadPrefs().value("published_blog_url").toString());
    unifiedForm->addRow("Published blog URL:",unifiedUrl);
    unifiedLayout->addLayout(unifiedForm);
    auto *unifiedPrepare=new QPushButton("Prepare one Vibe session");
    auto *unifiedCopy=new QPushButton("Copy single launch command");
    auto *unifiedRefresh=new QPushButton("Refresh next step");
    auto *unifiedRow=new QHBoxLayout; unifiedRow->addWidget(unifiedPrepare);unifiedRow->addWidget(unifiedCopy);unifiedRow->addWidget(unifiedRefresh);unifiedLayout->addLayout(unifiedRow);
    auto *unifiedOutput=new QPlainTextEdit;
    unifiedOutput->setReadOnly(true);unifiedLayout->addWidget(unifiedOutput);
    const int unifiedTabIndex=tabs->addTab(unifiedTab,"Vibe Workspace");
    tabs->setTabToolTip(unifiedTabIndex,"Guided Vibe publishing and Moltbook draft workflow");
    const QString unifiedRoot=QDir::homePath()+"/.oct9-recallforge-cache/unified-vibe";
    const QString unifiedScript="/tmp/oct9recall/unified-vibe-run.sh";
    const auto prepareUnified=[=](){
        const QString docPath=unifiedRoot+"/vibe-instructions.md";
        const QString summary=QDir::homePath()+"/.tb/blog summary.md";
        const bool hasSummary=QFileInfo(summary).isFile() && !QFileInfo(summary).isSymLink() && QFileInfo(summary).size()>0;
        QByteArray instructions;
        const QString privateTemplate =
            QDir::homePath() +
            "/.local/state/recallforge/private-templates/unified-vibe-instructions.md";

        const QFileInfo templateInfo(privateTemplate);
        if (!templateInfo.isFile() || templateInfo.isSymLink() ||
            !templateInfo.isReadable() ||
            templateInfo.size() <= 0 ||
            templateInfo.size() > 128 * 1024 ||
            !(templateInfo.permissions() & QFileDevice::ReadOwner) ||
            (templateInfo.permissions() &
                (QFileDevice::ReadGroup | QFileDevice::ReadOther))) {
            unifiedOutput->setPlainText(
                "Private Vibe instructions are missing or unsafe. "
                "No Vibe launcher was generated.");
            return;
        }

        QFile templateFile(privateTemplate);
        if (!templateFile.open(QIODevice::ReadOnly)) {
            unifiedOutput->setPlainText(
                "Cannot read private Vibe instructions. "
                "No Vibe launcher was generated.");
            return;
        }

        instructions = templateFile.readAll();

        if (instructions.isEmpty() || instructions.size() > 128 * 1024 ||
            !instructions.contains("## HIGHEST PRIORITY: TEST/LIVE BOUNDARY")) {
            unifiedOutput->setPlainText(
                "Private Vibe instructions failed validation. "
                "No Vibe launcher was generated.");
            return;
        }
        QJsonObject learning=loadPrefs().value("learning").toObject();
        learning.insert("review_limit",unifiedLimit->value());
        if(!savePrefs({{"learning",learning},{"published_blog_url",unifiedUrl->text().trimmed()}})) {
            unifiedOutput->setPlainText("Could not save private settings.json. No launcher generated.");return;
        }
        if(!safeMkpath(unifiedRoot) || !safeMkpath("/tmp/oct9recall")) {
            unifiedOutput->setPlainText("Unsafe or inaccessible launcher directory.");return;
        }
        if(!safeWrite(docPath,instructions)) {unifiedOutput->setPlainText("Cannot write Vibe instructions.");return;}
        const QByteArray shell=R"SHELL(#!/usr/bin/env bash
# Surfs up! My computer can crash. Keep checkpoints and report time/breaks before work.
set -euo pipefail
umask 077
root="$HOME/.oct9-recallforge-cache/unified-vibe"
file="$root/vibe-instructions.md"
[[ -f "$file" && ! -L "$file" ]] || { echo 'Unified Vibe instructions are missing.' >&2; exit 2; }
echo 'Surfs up! One RecallForge Vibe session. Checkpoint each stage.'
echo 'Deadline: check t14-finish deadline; do not force a new deadline after 8 PM.'
if command -v t14-finish >/dev/null 2>&1; then t14-finish deadline || true; fi
echo 'Break: safe to stop at any completed stage; checkpoints support restarting.'
helpdir="$HOME/.local/state/recallforge/help"
if [[ -f "$helpdir/RecallForge-Publishing-Help-Library.json" && ! -L "$helpdir/RecallForge-Publishing-Help-Library.json" ]]; then
  echo 'Publishing help JSON available for Vibe if additional fresh topics are needed.'
elif [[ -f "$helpdir/RecallForge-Publishing-Help-Library.md" && ! -L "$helpdir/RecallForge-Publishing-Help-Library.md" ]]; then
  echo 'Publishing help Markdown available for Vibe if additional fresh topics are needed.'
else
  echo 'Optional publishing help absent; Vibe can continue with original study concepts.'
fi
logger -t recallforge-vibe -- 'UNIFIED VIBE SESSION START' 2>/dev/null || true
cd "$root"
# Preflight is owner-only and optional, installed outside the public project.
preflight="$HOME/.local/state/recallforge/vibe-preflight.sh"
if [[ -f "$preflight" && ! -L "$preflight" ]]; then
  bash "$preflight" || echo 'Nonfatal SSH preflight problem recorded; starting Vibe anyway.'
else
  echo 'Optional Vibe preflight not installed; starting Vibe without SSH precheck.'
fi
if command -v vibe >/dev/null 2>&1; then
  exec vibe 'Read ./vibe-instructions.md completely and execute its stages in this ONE session. Begin with read-only local checks. Ask before any remote write.'
elif command -v mistral-vibe >/dev/null 2>&1; then
  exec mistral-vibe 'Read ./vibe-instructions.md completely and execute its stages in this ONE session. Begin with read-only local checks. Ask before any remote write.'
else
  echo 'Mistral Vibe executable not found.' >&2; exit 127
fi
)SHELL";
        if(!safeWrite(unifiedScript,shell)) {unifiedOutput->setPlainText("Could not create launcher.");return;}
        QFile::setPermissions(unifiedScript,QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
        unifiedOutput->setPlainText(QString("Single launcher ready: bash %1\nMistake summary: %2\nIf missing, the unified workflow uses safe fallback study topics and NEVER forges incorrect answers.\nReview maximum: %3\nNo Vibe process was launched by the GUI.").arg(unifiedScript,hasSummary?"available":"not available",QString::number(unifiedLimit->value())));
    };
    connect(unifiedPrepare,&QPushButton::clicked,this,[=](){
        prepareUnified();
        nextAction->setText("NEXT → Copy single launch command, then run it in your terminal");
        if (*guidanceEnabled) blueButton(unifiedCopy,true);
        blueButton(unifiedPrepare,false);
        tabs->setCurrentIndex(unifiedTabIndex);
    });
    // /tmp may be cleared across boots. Restore the launcher from durable instructions
    // by running the same safe local generator, never running Vibe itself.
    if(!QFileInfo(unifiedScript).isFile() || QFileInfo(unifiedScript).isSymLink()) prepareUnified();
    connect(unifiedCopy,&QPushButton::clicked,this,[=](){
        if(!QFileInfo(unifiedScript).isFile() || QFileInfo(unifiedScript).isSymLink()) prepareUnified();
        if(!QFileInfo(unifiedScript).isFile() || QFileInfo(unifiedScript).isSymLink()) return;
        QApplication::clipboard()->setText("bash "+unifiedScript);
        unifiedOutput->appendPlainText("Copied: bash "+unifiedScript);
        nextAction->setText("NEXT → Paste the copied command in your terminal. After Vibe finishes, return here and click Refresh next step.");
        blueButton(unifiedCopy,false);
    });

    connect(unifiedRefresh,&QPushButton::clicked,this,[=](){
        const QString file=unifiedRoot+"/stage-status.json";
        QFile f(file);
        if (!QFileInfo(file).isFile() || QFileInfo(file).isSymLink() || !f.open(QIODevice::ReadOnly) || f.size()>16384) {
            nextAction->setText("NEXT → Prepare or run the single Vibe session (no stage status available yet)"); return;
        }
        auto d=QJsonDocument::fromJson(f.readAll());
        if(!d.isObject()) { nextAction->setText("NEXT → Stage status is invalid. Review Vibe output.");return; }
        const auto step=d.object().value("next_step").toString();
        const QMap<QString,QString> labels={{"prepare","Prepare one Vibe session"},{"run_vibe","Run the Vibe launch command in your terminal"},{"approve_blog","Review and approve your learning blog"},{"publish_blog","Publish the approved article on j03.page"},{"enter_url","Paste the published j03.page article URL above"},{"review_moltbook","Review the next Moltbook recommendation in Vibe"},{"complete","Completed — prepare another learning session when ready"}};
        nextAction->setText("NEXT → "+labels.value(step,"Review the last Vibe output for your next step"));
        if(step=="enter_url" && *guidanceEnabled) { unifiedUrl->setFocus(); unifiedUrl->setStyleSheet("QLineEdit { border: 2px solid #60a5fa; }"); }
        else unifiedUrl->setStyleSheet("");
    });
    // Publishing results are read from a non-sensitive Vibe-produced JSON file.
    // No guesses, private questions or fabricated URL are put into this view.
    auto *resultsTab=new QWidget;
    auto *resultsLayout=new QVBoxLayout(resultsTab);
    auto *resultsRefresh=new QPushButton("Refresh publishing results");
    resultsRefresh->setToolTip("Reload the locally saved Vibe results");
    auto *resultsTop=new QHBoxLayout;
    resultsTop->addWidget(resultsRefresh); resultsTop->addStretch(); resultsLayout->addLayout(resultsTop);
    auto *resultsText=new QPlainTextEdit;
    resultsText->setReadOnly(true);
    resultsText->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    resultsText->setPlaceholderText("Waiting for the unified Vibe workflow to save publishing-results.json.");
    resultsLayout->addWidget(resultsText);
    auto *resultsCopy=new QPushButton("Copy all publishing text");
    resultsLayout->addWidget(resultsCopy);
    const int resultsIndex=tabs->addTab(resultsTab,"Publishing Results");
    const QString resultsFile=unifiedRoot+"/publishing-results.json";
    const auto isTesting=[=](){return loadPrefs().value("workflow").toObject().value("mode").toString("testing")!="live";};
    const QString testResultsFile=unifiedRoot+"/testing/publishing-results.json";

    // Compact snapshots, no exam questions, keys or private engine material.
    const QString archiveDir=unifiedRoot+"/publishing-archive";
    const QString archiveIndex=archiveDir+"/index.json";
    const QString reservePath=QDir::homePath()+"/.local/state/recallforge/help/recallforge-180-blogs.json";
    const auto freeSpaceOK=[=]()->bool {
        QStorageInfo disk(QDir::homePath()); disk.refresh();
        return disk.isValid() && disk.isReady() && disk.bytesTotal()>0 &&
            (disk.bytesAvailable()>=15LL*1024*1024*1024 || double(disk.bytesAvailable())/double(disk.bytesTotal())>=0.40);
    };
    const auto words=[](const QJsonObject &o)->QStringList {
        QString a=(o.value("title").toString()+" "+o.value("paragraph_1").toString()+" "+o.value("paragraph_2").toString()).toCaseFolded();
        return a.split(QRegularExpression("[^\\p{L}\\p{N}]+"),Qt::SkipEmptyParts);
    };
    const auto overlap=[=](const QJsonObject &a,const QJsonObject &b)->double {
        auto wa=words(a),wb=words(b);
        if(wa.isEmpty() || wb.isEmpty()) return 0.0;
        QMap<QString,int> count;for (const auto &w:wa) ++count[w];
        int common=0;for(const auto &w:wb) if(count.value(w)>0){++common;--count[w];}
        return double(common)/double(std::min(wa.size(),wb.size()));
    };
    const auto archiveAndRotate=[=]()->QString {
        if(isTesting()) return "TESTING MODE: live publishing archive untouched; read the isolated test preview.";
        if (!freeSpaceOK()) return "Storage protection: less than 15 GiB AND less than 40% free; no publishing archive changes.";
        if(QFileInfo(archiveIndex).isSymLink() || QFileInfo(reservePath).isSymLink() || QFileInfo(resultsFile).isSymLink()) return "Unsafe symlink; archive skipped.";
        QFile f(resultsFile);if(!f.open(QIODevice::ReadOnly)||f.size()>131072) return {};
        const auto d=QJsonDocument::fromJson(f.readAll()); if(!d.isObject()) return {};
        QJsonObject incoming=d.object();if(incoming.value("title").toString().isEmpty() || words(incoming).size()<40) return {};
        QJsonArray entries;
        QFile hf(archiveIndex);
        if(hf.open(QIODevice::ReadOnly) && hf.size()<=8*1024*1024) entries=QJsonDocument::fromJson(hf.readAll()).object().value("records").toArray();
        // Also check the existing Qt publishing ledger, not only the new Vibe snapshot archive.
        QJsonArray known=entries;
        QFile previousLedger(QDir::homePath()+"/.local/state/recallforge/publishing-history.json");
        if(previousLedger.open(QIODevice::ReadOnly) && previousLedger.size()<=8*1024*1024) {
            const auto oldDoc=QJsonDocument::fromJson(previousLedger.readAll());
            for(const auto &item:oldDoc.object().value("records").toArray()) known.append(item);
        }
        // Re-opening RecallForge or pressing Refresh on the SAME accepted result
        // must not consume another reserve item. Only a different incoming draft
        // can be flagged as an 85% duplicate.
        bool alreadyArchived=false;
        for(const auto &v:entries) {
            const auto old=v.toObject();
            if(old.value("title")==incoming.value("title") && overlap(incoming,old)>0.999) {alreadyArchived=true;break;}
        }
        bool similar=false;
        if(!alreadyArchived) for(const auto &v:known) if(overlap(incoming,v.toObject())>=.85){similar=true;break;}
        if(similar){
            QFile rf(reservePath);
            if(!rf.open(QIODevice::ReadOnly) || rf.size()>4*1024*1024) return "Repeated blog found, but reserve library unavailable; original preserved.";
            const auto lib=QJsonDocument::fromJson(rf.readAll()).object().value("entries").toArray();
            QJsonObject selected;
            for(const auto &v:lib){auto candidate=v.toObject();bool used=false;
                for(const auto &existing:known) if(overlap(candidate,existing.toObject())>=.85 || candidate.value("title")==existing.toObject().value("title")){used=true;break;}
                if(!used && overlap(candidate,incoming)<.85){selected=candidate;break;}
            }
            if(selected.isEmpty()) return "All reserve articles used or unavailable; old Vibe draft preserved.";
            // Preserve the rejected draft before changing the current publishing result.
            QJsonObject rejection={{"reason","85% word overlap"},{"received_at",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},{"title",incoming.value("title")}};
            if(!safeMkpath(archiveDir) || !safeWrite(archiveDir+"/last-rejected.json",QJsonDocument(rejection).toJson())) return "Unable to checkpoint duplicate; original preserved.";
            // Never import old social text containing a URL for a different article.
            selected.insert("facebook",selected.value("facebook").toString());
            selected.insert("linkedin",selected.value("linkedin").toString());
            if(!safeWrite(resultsFile,QJsonDocument(selected).toJson(QJsonDocument::Indented))) return "Unable to save reserve article; original preserved.";
            incoming=selected;
        }
        bool recorded=false;
        for(const auto &e:entries){const auto o=e.toObject();if(o.value("title")==incoming.value("title") && overlap(o,incoming)>.98){recorded=true;break;}}
        if(!recorded){
            // Save only needed article/writing fields. No source question text or account details.
            QJsonObject record;
            for(const QString &key:{"title","paragraph_1","paragraph_2","facebook","linkedin","spoken"}) record.insert(key,incoming.value(key));
            record.insert("recorded_at",QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            entries.append(record);
            if(!safeMkpath(archiveDir)||!safeWrite(archiveIndex,QJsonDocument(QJsonObject{{"schema",1},{"records",entries}}).toJson(QJsonDocument::Indented))) return "Unable to archive publishing result.";
        }
        return similar?"Similar Vibe article (>=85% word overlap) replaced with an unused reserve article. Original content is preserved in private history where available.":QString();
    };
    const auto refreshResults=[=](){
        const QString displayFile=isTesting()?testResultsFile:resultsFile;
        QFile f(displayFile);
        if (QFileInfo(displayFile).isSymLink() || !f.open(QIODevice::ReadOnly) || f.size()>131072) {
            resultsText->setPlainText("No publishing results available at: "+displayFile+"\nCheck Testing/Live mode, then use Vibe Workspace > Prepare one Vibe session and Refresh.\nSocial drafts can use [INSERT J03.PAGE BLOG LINK].");
            return;
        }
        QJsonParseError e;
        auto d=QJsonDocument::fromJson(f.readAll(),&e);
        if(e.error!=QJsonParseError::NoError || !d.isObject()) {
            resultsText->setPlainText("Results file is not valid JSON. Ask Vibe to recreate publishing-results.json.");return;
        }
        const auto o=d.object();
        QString title=o.value("title").toString().trimmed();
        QString first=o.value("paragraph_1").toString().trimmed();
        QString second=o.value("paragraph_2").toString().trimmed();
        QString facebook=o.value("facebook").toString().trimmed();
        QString linkedin=o.value("linkedin").toString().trimmed();
        QString spoken=o.value("spoken").toString().trimmed();
        QString url=loadPrefs().value("published_blog_url").toString().trimmed();
        // A URL for a different article is never pasted into a fresh result.
        QString slug=title.toCaseFolded().simplified();slug.replace(' ', '-');
        if(!url.contains(slug,Qt::CaseInsensitive)) url.clear();
        const QString linkMarker="[INSERT J03.PAGE BLOG LINK]";
        const QString missing="Pending — run Vibe Workspace to generate this section.";
        // Draft text remains copyable without a published URL. Never attach an old cycle's URL.
        if(url.isEmpty()) {
            if(facebook.isEmpty()) facebook="Read my new cybersecurity learning article: "+(title.isEmpty()?QString("New Security+ Lesson"):title)+". I explain the concepts in original, practical language as I prepare for Security+. Read it here: "+linkMarker+"\n\n#Cybersecurity #SecurityPlus #RecallForge";
            if(linkedin.isEmpty()) linkedin="I'm preparing for CompTIA Security+ and developing RecallForge to turn study topics into clear, practical explanations. My new article, "+(title.isEmpty()?QString("New Security+ Lesson"):title)+", shares what I'm learning. Read it here: "+linkMarker+"\n\n#Cybersecurity #CompTIA #SecurityPlus #RecallForge";
        } else {
            if(facebook.contains(linkMarker)) facebook.replace(linkMarker,url);
            if(linkedin.contains(linkMarker)) linkedin.replace(linkMarker,url);
        }
        const QString staleUrl=QStringLiteral("https://j03.page/2026/10/10/turning-mistakes-into-understanding/");
        if(title.toCaseFolded()!=QStringLiteral("Turning Mistakes Into Understanding").toCaseFolded()) {
            facebook.replace(staleUrl,linkMarker);linkedin.replace(staleUrl,linkMarker);
        }
        resultsText->setPlainText((title.isEmpty()?missing:title)+"\n\n"+
            (first.isEmpty()?missing:first)+"\n\n"+(second.isEmpty()?missing:second)+
            "\n\nFACEBOOK\n"+(facebook.isEmpty()?missing:facebook)+
            "\n\nLINKEDIN\n"+(linkedin.isEmpty()?missing:linkedin)+
            "\n\nSPOKEN TEXT\n"+(spoken.isEmpty()?missing:spoken));
    };
    // Private, durable publishing ledger; never include bank content. Each result has
    // independently confirmed completion flags, so a new blog cannot erase older work.
    const QString publishingHistory=stateRoot+"/publishing-history.json";
    auto *doneBlog=new QCheckBox("Blog published on j03.page");
    auto *doneFacebook=new QCheckBox("Facebook published");
    auto *doneLinkedin=new QCheckBox("LinkedIn published");
    auto *doneSpoken=new QCheckBox("Cowles Mountain recording completed");
    auto *completionRow=new QHBoxLayout;
    for(auto *check : {doneBlog,doneFacebook,doneLinkedin,doneSpoken}) completionRow->addWidget(check);
    resultsLayout->addLayout(completionRow);
    auto *resultsNotice=new QLabel;
    resultsNotice->setWordWrap(true);
    resultsLayout->addWidget(resultsNotice);
    const auto loadHistory=[=]()->QJsonObject {
        QFile f(publishingHistory);
        if(QFileInfo(publishingHistory).isSymLink() || !f.open(QIODevice::ReadOnly) || f.size()>1024*1024) return {};
        const auto d=QJsonDocument::fromJson(f.readAll());
        return d.isObject()?d.object():QJsonObject{};
    };
    const auto loadCurrent=[=]()->QJsonObject {
        const QString displayFile=isTesting()?testResultsFile:resultsFile;
        QFile f(displayFile);
        if(QFileInfo(displayFile).isSymLink() || !f.open(QIODevice::ReadOnly) || f.size()>131072) return {};
        const auto d=QJsonDocument::fromJson(f.readAll());
        return d.isObject()?d.object():QJsonObject{};
    };
    const auto resultId=[=](const QJsonObject &o)->QString {
        const auto data=(o.value("title").toString().trimmed().toCaseFolded()+"\n"+
            o.value("paragraph_1").toString().trimmed()+"\n"+o.value("paragraph_2").toString().trimmed()).toUtf8();
        return data.trimmed().isEmpty()?QString():QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex());
    };
    const auto refreshLedger=[=](){
        if(isTesting()){for(auto *c:{doneBlog,doneFacebook,doneLinkedin,doneSpoken}){QSignalBlocker b(c);c->setChecked(false);c->setEnabled(false);}resultsNotice->setText("TESTING MODE: preview only; live milestones and publishing history unchanged.");return;}
        const auto current=loadCurrent();const auto id=resultId(current);
        const auto history=loadHistory();const auto records=history.value("records").toArray();
        QJsonObject found;
        for(const auto &v:records) if(v.toObject().value("id").toString()==id) {found=v.toObject();break;}
        const auto completed=found.value("completed").toObject();
        const QList<QPair<QCheckBox*,QString>> entries={{doneBlog,"blog"},{doneFacebook,"facebook"},{doneLinkedin,"linkedin"},{doneSpoken,"spoken"}};
        for(const auto &entry:entries) {QSignalBlocker b(entry.first);entry.first->setChecked(completed.value(entry.second).toBool(false));entry.first->setEnabled(!id.isEmpty());}
        const auto url=loadPrefs().value("published_blog_url").toString().trimmed();
        if(id.isEmpty()) resultsNotice->setText("No complete publishing draft yet. Prepare one Vibe session.");
        else if(url.isEmpty()) resultsNotice->setText("NEXT → Copy your social drafts now; replace [INSERT J03.PAGE BLOG LINK] when you publish, then record the real URL in Vibe Workspace.");
        else if(current.value("facebook").toString().trimmed().isEmpty() || current.value("linkedin").toString().trimmed().isEmpty()) resultsNotice->setText("NEXT → Facebook or LinkedIn is missing. Prepare Vibe Workspace to fill only those sections using your saved URL.");
        else resultsNotice->setText("Mark each item complete after you publish or record it. Completed entries persist in private history.");
    };
    const auto saveCompletion=[=](){
        if(isTesting()){resultsNotice->setText("Testing Mode: cannot mark live publishing milestones complete.");return;}
        const auto current=loadCurrent();const auto id=resultId(current);
        if(id.isEmpty()) {resultsNotice->setText("Cannot record completion without a saved blog draft.");return;}
        QJsonObject history=loadHistory();QJsonArray records=history.value("records").toArray();
        QJsonObject record;int index=-1;
        for(int i=0;i<records.size();++i) if(records.at(i).toObject().value("id").toString()==id){index=i;record=records.at(i).toObject();break;}
        QJsonObject completed=record.value("completed").toObject();
        completed.insert("blog",doneBlog->isChecked());completed.insert("facebook",doneFacebook->isChecked());
        completed.insert("linkedin",doneLinkedin->isChecked());completed.insert("spoken",doneSpoken->isChecked());
        record.insert("id",id);record.insert("title",current.value("title"));
        record.insert("paragraph_1",current.value("paragraph_1"));record.insert("paragraph_2",current.value("paragraph_2"));
        record.insert("facebook",current.value("facebook"));record.insert("linkedin",current.value("linkedin"));
        record.insert("spoken",current.value("spoken"));record.insert("completed",completed);
        const QString url=loadPrefs().value("published_blog_url").toString().trimmed();
        if(!url.isEmpty()) record.insert("published_blog_url",url);
        record.insert("updated_at",QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        if(index>=0) records.replace(index,record);else records.append(record);
        history.insert("schema",1);history.insert("records",records);
        if(!safeMkpath(stateRoot)||!safeWrite(publishingHistory,QJsonDocument(history).toJson(QJsonDocument::Indented)))
            resultsNotice->setText("Could not save private publishing history. Completion not recorded.");
        else refreshLedger();
    };
    for(auto *check:{doneBlog,doneFacebook,doneLinkedin,doneSpoken})
        connect(check,&QCheckBox::toggled,this,[=](bool){saveCompletion();});
    connect(resultsRefresh,&QPushButton::clicked,this,[=](){const QString notice=archiveAndRotate();refreshResults();refreshLedger();if(!notice.isEmpty())resultsNotice->setText(notice);});
    connect(resultsCopy,&QPushButton::clicked,this,[=](){QApplication::clipboard()->setText(resultsText->toPlainText());});
    {const QString notice=archiveAndRotate();refreshResults();refreshLedger();if(!notice.isEmpty())resultsNotice->setText(notice);}
    // Separate, read-only writing library; never an automatic Vibe result.
    auto *libraryTab=new QWidget;
    auto *libraryLayout=new QVBoxLayout(libraryTab);
    auto *libraryInfo=new QLabel("Independent Security+ writing library. 100 original optional writing sets; private history shown separately. Not exam questions or an exam answer key. The library does not change Vibe's drafts.");
    libraryInfo->setWordWrap(true);libraryLayout->addWidget(libraryInfo);
    auto *librarySearch=new QLineEdit;
    librarySearch->setPlaceholderText("Filter by title or topic");libraryLayout->addWidget(librarySearch);
    auto *librarySelector=new QComboBox;libraryLayout->addWidget(librarySelector);
    auto *libraryText=new QPlainTextEdit;libraryText->setReadOnly(true);libraryLayout->addWidget(libraryText);
    auto *libraryCopy=new QPushButton("Copy selected writing set");libraryLayout->addWidget(libraryCopy);
    const QString externalLibrary=QDir::homePath()+"/.local/state/recallforge/help/recallforge-writing-library-100.json";
    const QString externalHistory=QDir::homePath()+"/.local/state/recallforge/publishing-history.json";
    auto *libraryItems=new QList<QJsonObject>;
    const auto reloadLibrary=[=](){
        libraryItems->clear();
        const QString bundledLibrary =
            QDir::cleanPath(
                QCoreApplication::applicationDirPath()
                + "/../share/recallforge/recallforge-writing-library-100.json");
        const QString libraryPath =
            QFileInfo(externalLibrary).isFile()
                ? externalLibrary : bundledLibrary;
        QFile lib(libraryPath);
        if(!QFileInfo(libraryPath).isSymLink() && lib.open(QIODevice::ReadOnly) && lib.size()<2*1024*1024){
            const QJsonDocument doc=QJsonDocument::fromJson(lib.readAll());
            for(const auto &v:doc.object().value("entries").toArray()) if(v.isObject()) libraryItems->append(v.toObject());
        }
        // Approved/past work is independent of the optional library.
        QFile hist(externalHistory);
        if(!QFileInfo(externalHistory).isSymLink() && hist.open(QIODevice::ReadOnly) && hist.size()<2*1024*1024){
            QJsonDocument doc=QJsonDocument::fromJson(hist.readAll());
            for(const auto &v:doc.object().value("records").toArray()) if(v.isObject()){
                QJsonObject o=v.toObject();o.insert("_history",true);libraryItems->append(o);
            }
        }
        librarySelector->clear();
        const QString filter=librarySearch->text().trimmed();
        for(int i=0;i<libraryItems->size();++i){
            const auto obj=libraryItems->at(i);
            const QString title=obj.value("title").toString();
            const QString topic=obj.value("topic").toString();
            if(!filter.isEmpty() && !title.contains(filter,Qt::CaseInsensitive) && !topic.contains(filter,Qt::CaseInsensitive)) continue;
            librarySelector->addItem((obj.value("_history").toBool()?"Past: ":"Library: ")+title,i);
        }
        if(librarySelector->count()==0) libraryText->setPlainText("No writing sets available. The optional private writing library is not included in public releases. Past writing appears only when recorded locally.");
    };
    const auto showLibrary=[=](){
        const int idx=librarySelector->currentIndex()>=0 ? librarySelector->currentData().toInt() : -1;
        if(idx<0 || idx>=libraryItems->size()) return;
        const auto o=libraryItems->at(idx);
        const QString m="[INSERT J03.PAGE BLOG LINK]";
        QString fb=o.value("facebook").toString();QString li=o.value("linkedin").toString();
        if(!o.value("_history").toBool()) {if(!fb.contains(m)) fb+="\n"+m; if(!li.contains(m)) li+="\n"+m;}
        libraryText->setPlainText(o.value("title").toString()+"\n\n"+
            o.value("paragraph_1").toString()+"\n\n"+o.value("paragraph_2").toString()+
            "\n\nFACEBOOK\n"+fb+"\n\nLINKEDIN\n"+li+
            "\n\nSPOKEN TEXT\n"+o.value("spoken").toString());
    };
    connect(librarySearch,&QLineEdit::textChanged,this,[=](const QString&){reloadLibrary();showLibrary();});
    connect(librarySelector,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int){showLibrary();});
    connect(libraryCopy,&QPushButton::clicked,this,[=](){QApplication::clipboard()->setText(libraryText->toPlainText());});
    auto *libraryRefresh=new QPushButton("Reload library and past writing");libraryLayout->insertWidget(1,libraryRefresh);
    connect(libraryRefresh,&QPushButton::clicked,this,[=](){reloadLibrary();showLibrary();});
    tabs->addTab(libraryTab,"Writing Index");
    reloadLibrary();showLibrary();
    auto *settingsTab=new QWidget;
    auto *settingsLayout=new QVBoxLayout(settingsTab);
    auto *modePicker=new QComboBox;
    modePicker->addItem("Testing — private previews, no remote publishing", "testing");
    modePicker->addItem("Live — remote work requires explicit approval", "live");
    modePicker->setCurrentIndex(loadPrefs().value("workflow").toObject().value("mode").toString("testing")=="live"?1:0);
    auto *modeNote=new QLabel("Changing the mode takes effect for Qt immediately. Prepare a NEW Vibe session after each change. Testing Mode prohibits ALL remote posts, even if prompted to approve one.");
    modeNote->setWordWrap(true);settingsLayout->addWidget(modeNote);
    settingsLayout->addWidget(modePicker);
    connect(modePicker,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int index){
        QJsonObject workflow=loadPrefs().value("workflow").toObject();
        workflow.insert("mode",index==1?"live":"testing");
        savePrefs({{"workflow",workflow}});
    });
    auto *settingsIntro=new QLabel("Version checks are optional. File safety, privacy checks, and protected-path restrictions remain active.");
    settingsIntro->setWordWrap(true); settingsLayout->addWidget(settingsIntro);
    auto *localVersionCheck=new QCheckBox("Check local component versions when RecallForge starts");
    auto *remoteVersionCheck=new QCheckBox("Check Raspberry Pi Web Companion compatibility before publishing");
    settingsLayout->addWidget(localVersionCheck);
    settingsLayout->addWidget(remoteVersionCheck);
    auto *settingsFeedback=new QLabel;
    auto *moltEnabled=new QCheckBox("Allow Vibe to consult Moltbook and suggest one draft per session (approval required)");
    moltEnabled->setChecked(loadPrefs().value("moltbook").toObject().value("enabled").toBool(false));
    settingsLayout->addWidget(moltEnabled);
    auto *kinFamilyEnabled=new QCheckBox("Moltbook: include historical KinMap family information (approval required)");
    auto *kinResearchEnabled=new QCheckBox("Allow Vibe to research documented historical family records");
    auto *kinRotateEnabled=new QCheckBox("Rotate among verified connected ancestors");
    const QJsonObject familyInitial=loadPrefs().value("moltbook").toObject().value("family_rotation").toObject();
    kinFamilyEnabled->setChecked(familyInitial.value("enabled").toBool(false));
    kinResearchEnabled->setChecked(familyInitial.value("research_enabled").toBool(false));
    kinRotateEnabled->setChecked(familyInitial.value("rotate_enabled").toBool(true));
    settingsLayout->addWidget(kinFamilyEnabled);
    settingsLayout->addWidget(kinResearchEnabled);
    settingsLayout->addWidget(kinRotateEnabled);
    auto *kinInfo=new QLabel("KinMap's state file may contain only display positions. Vibe must verify relationships and birth dates before proposing family details. It will skip excluded private people.");
    kinInfo->setWordWrap(true); settingsLayout->addWidget(kinInfo);
    const auto saveKinSettings=[=](){
        QJsonObject m=loadPrefs().value("moltbook").toObject();
        QJsonObject f=m.value("family_rotation").toObject();
        f.insert("enabled",kinFamilyEnabled->isChecked());
        f.insert("research_enabled",kinResearchEnabled->isChecked());
        f.insert("rotate_enabled",kinRotateEnabled->isChecked());
        // Preserve the privately configured starting person.
        // Never replace it when changing rotation preferences.
        f.insert("require_post_approval",true);
        f.insert("birth_before","1920-01-01");
        m.insert("family_rotation",f);m.insert("require_approval",true);
        settingsFeedback->setText(savePrefs({{"moltbook",m}}) ? "KinMap rotation settings saved. Prepare a new unified Vibe launcher to apply." : "Unable to save family rotation settings.");
    };
    connect(kinFamilyEnabled,&QCheckBox::toggled,this,[=](bool){saveKinSettings();});
    connect(kinResearchEnabled,&QCheckBox::toggled,this,[=](bool){saveKinSettings();});
    connect(kinRotateEnabled,&QCheckBox::toggled,this,[=](bool){saveKinSettings();});
    auto *finalized=new QCheckBox("Finalized — show blue next-step highlights and practice timing");
    finalized->setChecked(*guidanceEnabled);
    settingsLayout->addWidget(finalized);
    auto *finalizedInfo=new QLabel("When enabled: complete at least 10 minutes AND 25 confirmed answers before moving from Interactive Questions to Vibe Workspace. No interruption during an unanswered question.");
    finalizedInfo->setWordWrap(true);settingsLayout->addWidget(finalizedInfo);
    connect(finalized,&QCheckBox::toggled,this,[=](bool on){
        QJsonObject flow=loadPrefs().value("workflow").toObject();flow.insert("finalized",on);
        if(!savePrefs({{"workflow",flow}})){settingsFeedback->setText("Unable to save Finalized preference.");return;}
        *guidanceEnabled=on;nextAction->setVisible(on);studyGuide->setVisible(on);
        if(!on){studyTimer->invalidate();*answersThisSession=0; blueButton(unifiedPrepare,false);blueButton(unifiedCopy,false);tabs->tabBar()->setStyleSheet("");}
        else {studyTimer->invalidate();*answersThisSession=0;}
    });
    auto *moltPolicy=new QLabel("Moltbook posting always requires your approval. No private exam questions, answer keys, or unverified family claims are published automatically.");
    moltPolicy->setWordWrap(true);settingsLayout->addWidget(moltPolicy);
    settingsFeedback->setWordWrap(true);settingsLayout->addWidget(settingsFeedback);
    settingsLayout->addStretch();
    tabs->addTab(settingsTab,"Settings");
    const QJsonObject initialChecks=loadPrefs().value("version_checks").toObject();
    localVersionCheck->setChecked(initialChecks.value("local_enabled").toBool(true));
    remoteVersionCheck->setChecked(initialChecks.value("web_companion_enabled").toBool(true));
    const auto saveChecks=[=](){
        QJsonObject checks=loadPrefs().value("version_checks").toObject();
        checks.insert("local_enabled",localVersionCheck->isChecked());
        checks.insert("web_companion_enabled",remoteVersionCheck->isChecked());
        if (savePrefs({{"version_checks",checks}}))
            settingsFeedback->setText("Preferences saved. Local startup version-check changes take effect next launch. Remote publishing checks are applied by the separately run Vibe instructions.");
        else settingsFeedback->setText("Could not save settings. Check the private settings path.");
    };
    connect(localVersionCheck,&QCheckBox::toggled,this,[=](bool){saveChecks();});
    connect(remoteVersionCheck,&QCheckBox::toggled,this,[=](bool){saveChecks();});
    connect(moltEnabled,&QCheckBox::toggled,this,[=](bool enabled){
        QJsonObject m=loadPrefs().value("moltbook").toObject();m.insert("enabled",enabled);m.insert("require_approval",true);
        settingsFeedback->setText(savePrefs({{"moltbook",m}}) ? "Moltbook preference saved. Prepare a new Vibe session to apply." : "Could not save Moltbook preference.");
    });

    auto *viewTab=new QWidget;
    auto *viewLayout=new QVBoxLayout(viewTab);
    auto *viewIntro=new QLabel("Choose which tabs to display. Settings and View always stay accessible. Hidden tabs retain their saved information.");
    viewIntro->setWordWrap(true);viewLayout->addWidget(viewIntro);
    auto *viewFeedback=new QLabel; viewFeedback->setWordWrap(true);
    const int viewIndex=tabs->addTab(viewTab,"View");
    const QJsonObject viewPrefs=loadPrefs().value("view").toObject();
    const QJsonArray hidden=viewPrefs.contains("hidden_tabs") ? viewPrefs.value("hidden_tabs").toArray() : QJsonArray{ "Vibe Writing", "Learning to Writing", "Web Companion", "Study to Publishing" };
    // Qt6 setTabVisible leaves the widget alive and preserves the index.
    const auto applyVisibility=[=](const QString &name,bool display){
        for (int i=0;i<tabs->count();++i)
            if (tabs->tabText(i)==name && name!="Settings" && name!="View") tabs->setTabVisible(i,display);
    };
    for (int i=0;i<viewIndex;++i) {
        const QString name=tabs->tabText(i);
        auto *check=new QCheckBox(name);
        check->setChecked(!hidden.contains(name));
        viewLayout->addWidget(check);
        applyVisibility(name,check->isChecked());
        connect(check,&QCheckBox::toggled,this,[=](bool show){
            applyVisibility(name,show);
            QJsonObject view=loadPrefs().value("view").toObject();
            QJsonArray hiddenTabs=view.value("hidden_tabs").toArray();
            QJsonArray result;
            for(const auto &v:hiddenTabs) if(v.toString()!=name && !v.toString().isEmpty()) result.append(v);
            if(!show) result.append(name);
            view.insert("hidden_tabs",result);
            viewFeedback->setText(savePrefs({{"view",view}}) ? "View preferences saved." : "Unable to save view preferences.");
        });
    }
    auto *showAll=new QPushButton("Show all tabs");
    viewLayout->addWidget(showAll);
    connect(showAll,&QPushButton::clicked,this,[=](){
        for(int i=0;i<tabs->count();++i) tabs->setTabVisible(i,true);
        for(auto *check:viewTab->findChildren<QCheckBox*>()) { QSignalBlocker blocker(check);check->setChecked(true); }
        QJsonObject view=loadPrefs().value("view").toObject();view.insert("hidden_tabs",QJsonArray{});
        viewFeedback->setText(savePrefs({{"view",view}}) ? "All tabs shown and saved." : "Could not save view preferences.");
    });
    viewLayout->addWidget(viewFeedback); viewLayout->addStretch();

    // Make existing tab bodies scrollable, retaining tab names for View controls.
    for(int i=tabs->count()-1;i>=0;--i) {
        QWidget *page=tabs->widget(i);
        if(!page || !page->layout()) continue;
        const QString label=tabs->tabText(i);
        const bool wasVisible=tabs->isTabVisible(i);
        tabs->removeTab(i);
        auto *scroller=new QScrollArea;
        scroller->setWidgetResizable(true);
        scroller->setFrameShape(QFrame::NoFrame);
        scroller->setWidget(page);
        tabs->insertTab(i,scroller,label);
        tabs->setTabVisible(i,wasVisible);
    }
    if (auto *layout=qobject_cast<QVBoxLayout*>(quizTab_->layout())) layout->addWidget(studyGuide);
    const auto updateGuidance=[=](){
        if(!*guidanceEnabled){studyGuide->hide();tabs->tabBar()->setStyleSheet("");return;}
        const qint64 elapsed=studyTimer->isValid()?studyTimer->elapsed()/1000:0;
        const int secondsLeft=qMax<qint64>(0,600-elapsed);
        const int answersLeft=qMax(0,25-*answersThisSession);
        const bool done=elapsed>=600 && *answersThisSession>=25;
        studyGuide->show();
        studyGuide->setText(done ? "NEXT → Open Vibe Workspace. Study minimum reached." :
            QString("Interactive Questions: %1 confirmed answer(s). %2 answer(s) and %3:%4 remaining minimum. Both targets must be reached. Finish the current question first.")
            .arg(*answersThisSession).arg(answersLeft).arg(secondsLeft/60).arg(secondsLeft%60,2,10,QLatin1Char('0')));
        const QString name=done?"Vibe Workspace":"Interactive Questions";
        // Highlight the tab name without changing the current selection or interrupting an answer.
        tabs->tabBar()->setStyleSheet("QTabBar::tab { padding: 7px; } QTabBar::tab:selected { border-bottom: 3px solid #60a5fa; } ");
        for(int i=0;i<tabs->count();++i){if(tabs->tabText(i)==name)tabs->tabBar()->setTabTextColor(i,QColor("#3489ee"));else tabs->tabBar()->setTabTextColor(i,QColor());}
        blueButton(done?unifiedPrepare:nextQuestion_,true);
        if(done) blueButton(nextQuestion_,false); else blueButton(unifiedPrepare,false);
    };
    auto *guidanceTicker=new QTimer(this);
    connect(guidanceTicker,&QTimer::timeout,this,updateGuidance);
    guidanceTicker->start(1000);
    updateGuidance();
    logsProcess_ = new QProcess(this);
    statusProcess_ = new QProcess(this);
    connect(logsProcess_, &QProcess::errorOccurred, this,
        [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart)
                logs_->setPlainText(
                    "System Logs unavailable: the journal command could not start.");
        });
    connect(logsProcess_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        const QString out = QString::fromUtf8(logsProcess_->readAllStandardOutput());
        const QString err = QString::fromUtf8(logsProcess_->readAllStandardError());
        logs_->setPlainText(out.isEmpty() ? (err.isEmpty() ? "No journal entries available." : err) : out);
        if (exitStatus != QProcess::NormalExit || exitCode != 0) logs_->appendPlainText("\nJournal command ended with a failure.");
    });
    connect(statusProcess_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        QString out = QString::fromUtf8(statusProcess_->readAllStandardOutput());
        const QString err = QString::fromUtf8(statusProcess_->readAllStandardError());
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            statusRaw_->setPlainText("Backend status failed. " + err);
            progress_->setText("Unavailable: backend did not return a valid status."); updateDashboard(QString()); return;
        }
        statusRaw_->setPlainText(out);
        updateDashboard(out);
    });
    refreshStatus(); refreshLogs();
}
MainWindow::~MainWindow() { delete ui; }
void MainWindow::refreshLogs() {
    if (logsProcess_->state() != QProcess::NotRunning) return;
    if (QStandardPaths::findExecutable("journalctl").isEmpty()) {
        logs_->setPlainText(
            "System Logs are unavailable in this installation. "
            "Flatpak does not provide journalctl or access to the host system journal. "
            "RecallForge remains usable without this optional feature.");
        return;
    }
    logs_->setPlainText("Loading system logs…");
    logsProcess_->setProgram("journalctl");
    logsProcess_->setArguments({"-t", "testbank", "--no-pager", "--output=short-iso", "-n", "500"});
    logsProcess_->start();
}
void MainWindow::refreshStatus() {
    const QFileInfo engine(backendPath());
    if (!engine.exists() || !engine.isExecutable() || engine.canonicalFilePath().startsWith(QDir::homePath() + "/Projects/")) {
        backend_->setText("Private backend not installed or located in a prohibited project directory: " + backendPath());
        nextQuestion_->setEnabled(false);
        progress_->setText("Learning Status requires the optional private question backend.");
        statusRaw_->clear(); updateDashboard(QString()); return;
    }
    nextQuestion_->setEnabled(true);
    backend_->setText("Private backend detected outside Projects: " + engine.canonicalFilePath());
    if (statusProcess_->state() != QProcess::NotRunning) return;
    statusRaw_->setPlainText("Checking status…");
    statusProcess_->setProgram(engine.canonicalFilePath());
    statusProcess_->setArguments({"status"});
    statusProcess_->start();
}

void MainWindow::updateDashboard(const QString &output) {
    // The current private engine emits a human-readable status line.
    // Parse only aggregate counters; never access the private question collection.
    const QRegularExpression rx(R"(Total:\s*(\d+)\s*\|\s*Available:\s*(\d+)\s*\|\s*Retired:\s*(\d+))");
    const auto match = rx.match(output);
    bool valid = false;
    qint64 total = 0, available = 0, retired = 0;
    if (match.hasMatch()) {
        bool ok1=false,ok2=false,ok3=false;
        total=match.captured(1).toLongLong(&ok1);
        available=match.captured(2).toLongLong(&ok2);
        retired=match.captured(3).toLongLong(&ok3);
        valid = ok1 && ok2 && ok3 && total>0 && retired>=0 && available>=0 && available+retired==total;
    }
    if (!valid) {
        totalValue_->setText("—"); availableValue_->setText("—"); retiredValue_->setText("—");
        masteryBar_->setValue(0); masteryBar_->setFormat("Unavailable");
        progress_->setText(output.isEmpty()
            ? "Learning Status unavailable: optional private backend not installed or no status returned."
            : "Backend status format was not recognized. See backend output below.");
        return;
    }
    const int percent = static_cast<int>((retired * 100) / total);
    totalValue_->setText(QString::number(total));
    availableValue_->setText(QString::number(available));
    retiredValue_->setText(QString::number(retired));
    masteryBar_->setValue(percent);
    masteryBar_->setFormat(QString::number(percent) + "% retired");
    progress_->setText(QString("%1 of %2 questions permanently retired. %3 currently available.")
        .arg(retired).arg(total).arg(available));
}

QString MainWindow::safeBackend() const {
    const QFileInfo engine(backendPath());
    const QString canonical = engine.canonicalFilePath();
    const QString projects = QDir::homePath() + "/Projects";
    if (!engine.isFile() || !engine.isExecutable() || canonical.isEmpty() ||
        canonical == projects || canonical.startsWith(projects + "/")) return {};
    return canonical;
}
void MainWindow::requestQuestion() {
    if (questionProcess_->state() != QProcess::NotRunning || gradeProcess_->state() != QProcess::NotRunning) return;
    const QString engine = safeBackend();
    if (engine.isEmpty()) { questionResult_->setText("Private engine missing or in an invalid directory."); return; }
    // Abandoning a displayed, unanswered question does not count as an attempt.
    answered_ = true; currentQuestionId_.clear(); submitAnswer_->setEnabled(false);
    nextQuestion_->setEnabled(false); questionText_->setText("Retrieving question…");
    questionResult_->setText("Loading…");
    questionProcess_->start(engine, {"random", "--json"});
}
void MainWindow::displayQuestion(const QJsonObject &object) {
    const QString id = object.value("question_id").toString();
    const QString prompt = object.value("question").toString();
    const QJsonArray options = object.value("options").toArray();
    if (id.isEmpty() || prompt.isEmpty() || options.size() < 2 || options.size() > 64) {
        questionResult_->setText("Backend returned an invalid question."); return;
    }
    while (auto *item = choiceLayout_->takeAt(0)) { delete item->widget(); delete item; }
    delete choiceGroup_; choiceGroup_ = new QButtonGroup(this);
    multiple_ = object.value("multiple").toBool();
    choiceGroup_->setExclusive(!multiple_);
    for (int i=0; i<options.size(); ++i) {
        if (!options.at(i).isString()) { questionResult_->setText("Invalid answer choices."); return; }
        QAbstractButton *choice = multiple_ ? static_cast<QAbstractButton*>(new QCheckBox(options.at(i).toString()))
                                           : static_cast<QAbstractButton*>(new QRadioButton(options.at(i).toString()));
        choiceGroup_->addButton(choice, i+1);
        choiceLayout_->addWidget(choice);
    }
    currentQuestionId_ = id; optionCount_ = options.size(); answered_ = false;
    questionText_->setText(prompt + (multiple_ ? "\n\nSelect all correct answers." : "\n\nSelect one answer."));
    questionResult_->setText("Choose an answer, then select Submit Answer.");
    submitAnswer_->setEnabled(true);
}
void MainWindow::submitQuestion() {
    if (answered_ || currentQuestionId_.isEmpty() || gradeProcess_->state() != QProcess::NotRunning) return;
    QStringList chosen;
    for (int i=1; i<=optionCount_; ++i) {
        if (auto *button = choiceGroup_->button(i); button && button->isChecked()) chosen << QString::number(i);
    }
    if (chosen.isEmpty()) { questionResult_->setText("Select at least one answer first."); return; }
    const QString engine = safeBackend();
    if (engine.isEmpty()) { questionResult_->setText("Private engine unavailable."); return; }
    // Do not save questions or response text in public source/build directories or logs.
    submitAnswer_->setEnabled(false); nextQuestion_->setEnabled(false);
    questionResult_->setText("Submitting…");
    gradeProcess_->start(engine, {"grade", currentQuestionId_, chosen.join(",")});
}

void MainWindow::generateVibeLauncher() {
    const QString home = QDir::homePath();
    const QString source = home + "/.tb/blog summary.md";
    const QFileInfo info(source);
    const QString safeHome = QDir(home).canonicalPath();
    const QString canonical = info.canonicalFilePath();
    if (!info.isFile() || info.isSymLink() || canonical.isEmpty() ||
        !canonical.startsWith(safeHome + "/.tb/")) {
        vibeStatus_->setText("No safe private mistake summary found. Answer a question incorrectly twice and try again.");
        return;
    }
    QFile summary(source);
    if (!summary.open(QIODevice::ReadOnly) || summary.size() > 256 * 1024) {
        vibeStatus_->setText("Unable to read private summary, or summary exceeds 256 KiB."); return;
    }
    const QByteArray mistakes = summary.readAll();
    summary.close();
    if (mistakes.trimmed().isEmpty()) {
        vibeStatus_->setText("Private mistake summary is empty."); return;
    }
    const QString base = home + "/.oct9-recallforge-cache/vibe-writing";
    // Refuse an existing symlink, including a symlinked directory from another program.
    const QFileInfo baseInfo(base);
    if (baseInfo.isSymLink() || (baseInfo.exists() && !baseInfo.isDir())) {
        vibeStatus_->setText("Unsafe launcher directory. Refusing to write."); return;
    }
    if (!QDir().mkpath(base)) { vibeStatus_->setText("Cannot create private launcher directory."); return; }
    QFile::setPermissions(base, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    const QString promptPath = base + "/private-vibe-prompt.txt";
    const QString scriptDir = "/tmp/oct9recall";
    if (QFileInfo(scriptDir).isSymLink() || !QDir().mkpath(scriptDir)) {
        vibeStatus_->setText("Unable to create safe launcher directory."); return;
    }
    QFile::setPermissions(scriptDir, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    const QString scriptPath = scriptDir + "/vibe-writing-run.sh";
    if (QFileInfo(promptPath).isSymLink() || QFileInfo(scriptPath).isSymLink()) {
        vibeStatus_->setText("Unsafe launcher path. Refusing to overwrite a symbolic link."); return;
    }
    const QByteArray intro = R"VIBE(You are helping with an educational writing and recall exercise. The private notes below are for comprehension only; do not quote or reproduce their original question wording in public writing.
Explain in original language the underlying concept, why the selected response was wrong, and why the correct response is right. Do not invent details if the private notes are unclear.
Draft exactly two paragraphs for j03.page with a title containing exactly four words. Do not include the source question or multiple-choice options verbatim.
After drafting the blog, ask the user for the actual published blog URL. Do not invent one. Once it is supplied, produce a Facebook post, LinkedIn post, and a short, easy-to-read Cowles Mountain speaking script that teaches the concept and aids recall. Include the supplied URL in both posts. Do not publish anything automatically.

PRIVATE STUDY NOTES (not for publication):
)VIBE";
    QSaveFile prompt(promptPath);
    if (!prompt.open(QIODevice::WriteOnly) || prompt.write(intro) != intro.size() ||
        prompt.write(mistakes) != mistakes.size() || !prompt.commit()) {
        vibeStatus_->setText("Failed to write private Vibe prompt."); return;
    }
    QFile::setPermissions(promptPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    // Script contains *only* constant shell code; private study notes are in a separate restricted file.
    const QByteArray shell = R"SH(#!/usr/bin/env bash
# Surfs up! Checkpoints in journal; the private prompt is never logged.
set -euo pipefail
umask 077
logger -t recallforge-vibe -- 'START user-invoked Vibe educational writing' 2>/dev/null || true
prompt="$HOME/.oct9-recallforge-cache/vibe-writing/private-vibe-prompt.txt"
[[ -f "$prompt" && ! -L "$prompt" ]] || { echo 'Private Vibe prompt unavailable.' >&2; exit 2; }
if command -v vibe >/dev/null 2>&1; then
  echo 'Starting Vibe. Paste the private prompt file content into its conversation if prompted.'
  echo "Private prompt path: $prompt"
  cd "$HOME/.oct9-recallforge-cache/vibe-writing"
  exec vibe 'Read ./private-vibe-prompt.txt fully. Draft only; do not publish without approval.'
elif command -v mistral-vibe >/dev/null 2>&1; then
  echo 'Starting Mistral Vibe. Paste the private prompt file content into its conversation if prompted.'
  echo "Private prompt path: $prompt"
  cd "$HOME/.oct9-recallforge-cache/vibe-writing"
  exec mistral-vibe 'Read ./private-vibe-prompt.txt fully. Draft only; do not publish without approval.'
else
  echo 'Neither vibe nor mistral-vibe was found. Open your installed Vibe program manually and load the private prompt file.' >&2
  echo "Private prompt path: $prompt"
  exit 3
fi
)SH";
    QSaveFile script(scriptPath);
    if (!script.open(QIODevice::WriteOnly) || script.write(shell) != shell.size() || !script.commit()) {
        vibeStatus_->setText("Failed to write private launcher script."); return;
    }
    QFile::setPermissions(scriptPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    vibeStatus_->setText("Private prompt and launcher prepared. Run: bash /tmp/oct9recall/vibe-writing-run.sh. The launcher tells Vibe to read its instructions. No private data is stored in the project.");
}
