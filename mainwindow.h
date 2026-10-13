#pragma once
#include <QMainWindow>
#include <QProcess>
#include <QPlainTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QButtonGroup>
#include <QJsonObject>
#include <QVBoxLayout>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    Ui::MainWindow *ui;
    QPlainTextEdit *logs_ = nullptr;
    QPlainTextEdit *statusRaw_ = nullptr;
    QLabel *backend_ = nullptr;
    QLabel *progress_ = nullptr;
    QProgressBar *masteryBar_ = nullptr;
    QLabel *totalValue_ = nullptr;
    QLabel *availableValue_ = nullptr;
    QLabel *retiredValue_ = nullptr;
    QLabel *answersValue_ = nullptr;
    QLabel *backupValue_ = nullptr;
    QProcess *logsProcess_ = nullptr;
    QProcess *statusProcess_ = nullptr;
    QProcess *questionProcess_ = nullptr;
    QProcess *gradeProcess_ = nullptr;
    QWidget *quizTab_ = nullptr;
    QLabel *vibeStatus_ = nullptr;
    QLabel *questionText_ = nullptr;
    QLabel *questionResult_ = nullptr;
    QPushButton *nextQuestion_ = nullptr;
    QPushButton *submitAnswer_ = nullptr;
    QWidget *choiceContainer_ = nullptr;
    QVBoxLayout *choiceLayout_ = nullptr;
    QButtonGroup *choiceGroup_ = nullptr;
    QString currentQuestionId_;
    int optionCount_ = 0;
    bool multiple_ = false;
    bool answered_ = true;
    QString safeBackend() const;
    void generateVibeLauncher();
    void requestQuestion();
    void displayQuestion(const QJsonObject &object);
    void submitQuestion();
    void refreshLogs();
    void refreshStatus();
    void updateDashboard(const QString &output);
};
