#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class FindDialog;
class ReplaceDialog;
class QLabel;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // 关闭窗口时，如果有未保存的修改要先询问
    void closeEvent(QCloseEvent *event) override;

private slots:
    // ---- 文件 ----
    void newFile();
    void openFile();
    bool saveFile();
    bool saveAsFile();
    void exitApp();

    // ---- 编辑 ----
    void undoAction();
    void redoAction();
    void cutAction();
    void copyAction();
    void pasteAction();
    void selectAllAction();

    // ---- 格式 ----
    void changeFontSize();
    void changeFontColor();
    void changeBackgroundColor();
    void toggleWordWrap(bool checked);

    // ---- 视图 ----
    void toggleLineNumber(bool checked);
    void toggleToolBar(bool checked);
    void toggleStatusBar(bool checked);

    // ---- 查找 / 替换 ----
    void showFindDialog();
    void showReplaceDialog();
    void doFindNext();
    void doReplace();
    void doReplaceAll();

    // ---- 其它 ----
    void showAbout();
    void onModificationChanged(bool changed);
    void updateStatusBar();
    void updatePasteAction();
    void autoSave();

private:
    bool maybeSave();
    bool saveToFile(const QString &fileName);
    bool loadFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    bool findTextInEditor(const QString &text, bool caseSensitive, bool backward);
    void applyStandardIcons();
    void applyEditorFont();

    Ui::MainWindow *ui;
    QString m_currentFile;              // 当前文件完整路径，为空表示"还没保存过"
    FindDialog *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;
    QTimer *m_autoSaveTimer = nullptr;
    QLabel *m_statusLabel = nullptr;    // 状态栏左侧：行列号 / 字符数 / 修改状态
};

#endif // MAINWINDOW_H
