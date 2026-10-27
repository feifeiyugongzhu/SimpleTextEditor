#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QStyle>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTime>
#include <QTimer>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---------- 工具栏外观 ----------
    applyStandardIcons();
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->mainToolBar->setMovable(false);

    // 工具栏按钮用简洁文字（菜单里仍然显示“新建(N)”这种快捷键提示）
    ui->actionNew->setIconText(QStringLiteral("新建"));
    ui->actionOpen->setIconText(QStringLiteral("打开"));
    ui->actionSave->setIconText(QStringLiteral("保存"));
    ui->actionSaveAs->setIconText(QStringLiteral("另存为"));
    ui->actionUndo->setIconText(QStringLiteral("撤销"));
    ui->actionRedo->setIconText(QStringLiteral("恢复"));
    ui->actionCut->setIconText(QStringLiteral("剪切"));
    ui->actionCopy->setIconText(QStringLiteral("复制"));
    ui->actionPaste->setIconText(QStringLiteral("粘贴"));
    ui->actionFind->setIconText(QStringLiteral("查找"));
    ui->actionReplace->setIconText(QStringLiteral("替换"));
    ui->actionFontColor->setIconText(QStringLiteral("字体颜色"));
    ui->actionBackgroundColor->setIconText(QStringLiteral("背景色"));

    // ---------- 编辑器初始状态 ----------
    applyEditorFont();
    ui->editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    ui->editor->setLineNumberVisible(true);
    ui->editor->setTabStopDistance(4 * ui->editor->fontMetrics().horizontalAdvance(QLatin1Char(' ')));

    // ---------- 状态栏 ----------
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel);

    // ---------- 文件 ----------
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAsFile);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exitApp);

    // ---------- 编辑 ----------
    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undoAction);
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redoAction);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cutAction);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copyAction);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::pasteAction);
    connect(ui->actionSelectAll, &QAction::triggered, this, &MainWindow::selectAllAction);

    // ---------- 格式 ----------
    connect(ui->actionFontSize, &QAction::triggered, this, &MainWindow::changeFontSize);
    connect(ui->actionFontColor, &QAction::triggered, this, &MainWindow::changeFontColor);
    connect(ui->actionBackgroundColor, &QAction::triggered, this, &MainWindow::changeBackgroundColor);
    connect(ui->actionWrap, &QAction::toggled, this, &MainWindow::toggleWordWrap);

    // ---------- 视图 ----------
    connect(ui->actionLineNumber, &QAction::toggled, this, &MainWindow::toggleLineNumber);
    connect(ui->actionShowToolBar, &QAction::toggled, this, &MainWindow::toggleToolBar);
    connect(ui->actionShowStatusBar, &QAction::toggled, this, &MainWindow::toggleStatusBar);

    // ---------- 查找 / 替换 ----------
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // ---------- 帮助 ----------
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(ui->actionAboutQt, &QAction::triggered, this, &QApplication::aboutQt);

    // ---------- 编辑区状态同步 ----------
    connect(ui->editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(ui->editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::onModificationChanged);

    // 剪切 / 复制 / 撤销 / 恢复 的使能跟着编辑状态走
    connect(ui->editor, &QPlainTextEdit::copyAvailable, ui->actionCut, &QAction::setEnabled);
    connect(ui->editor, &QPlainTextEdit::copyAvailable, ui->actionCopy, &QAction::setEnabled);
    connect(ui->editor, &QPlainTextEdit::undoAvailable, ui->actionUndo, &QAction::setEnabled);
    connect(ui->editor, &QPlainTextEdit::redoAvailable, ui->actionRedo, &QAction::setEnabled);
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updatePasteAction);

    // 工具栏被直接关掉时，菜单里的勾选状态跟着变化
    connect(ui->mainToolBar, &QToolBar::visibilityChanged,
            ui->actionShowToolBar, &QAction::setChecked);

    // ---------- 定时自动保存（选做，每 60 秒）----------
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(60 * 1000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    m_autoSaveTimer->start();

    // ---------- 初始使能状态 ----------
    ui->actionCut->setEnabled(false);
    ui->actionCopy->setEnabled(false);
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    updatePasteAction();

    setCurrentFile(QString());
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ==================== 外观辅助 ====================

void MainWindow::applyStandardIcons()
{
    QStyle *s = style();
    ui->actionNew->setIcon(s->standardIcon(QStyle::SP_FileIcon));
    ui->actionOpen->setIcon(s->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actionSave->setIcon(s->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actionSaveAs->setIcon(s->standardIcon(QStyle::SP_DriveFDIcon));
    ui->actionExit->setIcon(s->standardIcon(QStyle::SP_DialogCloseButton));
    ui->actionUndo->setIcon(s->standardIcon(QStyle::SP_ArrowBack));
    ui->actionRedo->setIcon(s->standardIcon(QStyle::SP_ArrowForward));
    ui->actionFind->setIcon(s->standardIcon(QStyle::SP_FileDialogContentsView));
    ui->actionReplace->setIcon(s->standardIcon(QStyle::SP_FileDialogDetailedView));
    ui->actionAbout->setIcon(s->standardIcon(QStyle::SP_MessageBoxInformation));
}

void MainWindow::applyEditorFont()
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(12);
    ui->editor->setFont(font);
}

// ==================== 文件操作 ====================

void MainWindow::newFile()
{
    if (!maybeSave())
        return;
    ui->editor->clear();
    setCurrentFile(QString());
    statusBar()->showMessage(QStringLiteral("新建文件"), 2000);
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    const QString fileName = QFileDialog::getOpenFileName(
        this, QStringLiteral("打开文件"), QString(),
        QStringLiteral("文本文件 (*.txt *.cpp *.h *.md *.log);;所有文件 (*.*)"));
    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

bool MainWindow::saveFile()
{
    if (m_currentFile.isEmpty())
        return saveAsFile();
    return saveToFile(m_currentFile);
}

bool MainWindow::saveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, QStringLiteral("另存为"), m_currentFile,
        QStringLiteral("文本文件 (*.txt);;所有文件 (*.*)"));
    if (fileName.isEmpty())
        return false;
    return saveToFile(fileName);
}

bool MainWindow::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"),
                             QStringLiteral("无法写入文件：\n%1\n\n原因：%2")
                                 .arg(fileName, file.errorString()));
        return false;
    }

    const QByteArray data = ui->editor->toPlainText().toUtf8();
    const qint64 written = file.write(data);
    file.close();

    if (written != data.size()) {
        QMessageBox::warning(this, QStringLiteral("保存失败"),
                             QStringLiteral("文件只写入了一部分，请检查磁盘空间。"));
        return false;
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(QStringLiteral("已保存：%1").arg(QFileInfo(fileName).fileName()), 3000);
    return true;
}

bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("打开失败"),
                             QStringLiteral("无法打开文件：\n%1\n\n原因：%2")
                                 .arg(fileName, file.errorString()));
        return false;
    }

    // 异常情况处理：超大文件、疑似二进制文件
    const qint64 size = file.size();
    if (size > 10 * 1024 * 1024) {
        const auto ret = QMessageBox::question(
            this, QStringLiteral("文件较大"),
            QStringLiteral("该文件有 %1 MB，打开可能会比较慢，仍然继续吗？")
                .arg(QString::number(size / 1024.0 / 1024.0, 'f', 1)));
        if (ret != QMessageBox::Yes) {
            file.close();
            return false;
        }
    }

    const QByteArray data = file.readAll();
    file.close();

    if (data.contains('\0')) {
        const auto ret = QMessageBox::question(
            this, QStringLiteral("可能是二进制文件"),
            QStringLiteral("该文件包含二进制内容，用文本方式打开可能显示乱码，仍然继续吗？"));
        if (ret != QMessageBox::Yes)
            return false;
    }

    ui->editor->setPlainText(QString::fromUtf8(data));
    setCurrentFile(fileName);
    statusBar()->showMessage(QStringLiteral("已打开：%1").arg(QFileInfo(fileName).fileName()), 3000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFile = fileName;
    ui->editor->document()->setModified(false);
    setWindowModified(false);

    const QString shownName = fileName.isEmpty()
        ? QStringLiteral("无标题")
        : QFileInfo(fileName).fileName();
    setWindowTitle(QStringLiteral("%1[*] - 简单文本编辑器").arg(shownName));
}

bool MainWindow::maybeSave()
{
    if (!ui->editor->document()->isModified())
        return true;
    if (ui->editor->toPlainText().isEmpty())
        return true;

    const QString shownName = m_currentFile.isEmpty()
        ? QStringLiteral("无标题")
        : QFileInfo(m_currentFile).fileName();

    const auto ret = QMessageBox::warning(
        this, QStringLiteral("简单文本编辑器"),
        QStringLiteral("文档“%1”有未保存的修改，是否保存？").arg(shownName),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
        return saveFile();
    if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void MainWindow::exitApp()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// ==================== 编辑操作 ====================

void MainWindow::undoAction()
{
    ui->editor->undo();
}

void MainWindow::redoAction()
{
    ui->editor->redo();
}

void MainWindow::cutAction()
{
    ui->editor->cut();
}

void MainWindow::copyAction()
{
    ui->editor->copy();
}

void MainWindow::pasteAction()
{
    ui->editor->paste();
}

void MainWindow::selectAllAction()
{
    ui->editor->selectAll();
}

// ==================== 格式操作 ====================

void MainWindow::changeFontSize()
{
    bool ok = false;
    const int currentSize = ui->editor->font().pointSize();
    const int size = QInputDialog::getInt(this, QStringLiteral("字体大小"),
                                          QStringLiteral("请输入字号（6 ~ 72）："),
                                          currentSize > 0 ? currentSize : 12, 6, 72, 1, &ok);
    if (!ok)
        return;

    QFont font = ui->editor->font();
    font.setPointSize(size);
    ui->editor->setFont(font);
    statusBar()->showMessage(QStringLiteral("字号已设置为 %1").arg(size), 2000);
}

void MainWindow::changeFontColor()
{
    const QColor color = QColorDialog::getColor(ui->editor->palette().color(QPalette::Text),
                                                this, QStringLiteral("选择字体颜色"));
    if (!color.isValid())
        return;

    QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection()) {
        // 有选中：只改选中的文字
        QTextCharFormat format;
        format.setForeground(color);
        cursor.mergeCharFormat(format);
        ui->editor->mergeCurrentCharFormat(format);
    } else {
        // 没选中：改全局默认文字颜色
        QPalette palette = ui->editor->palette();
        palette.setColor(QPalette::Text, color);
        ui->editor->setPalette(palette);
    }
}

void MainWindow::changeBackgroundColor()
{
    const QColor color = QColorDialog::getColor(ui->editor->palette().color(QPalette::Base),
                                                this, QStringLiteral("选择背景颜色"));
    if (!color.isValid())
        return;

    QPalette palette = ui->editor->palette();
    palette.setColor(QPalette::Base, color);
    palette.setColor(QPalette::Active, QPalette::Base, color);
    palette.setColor(QPalette::Inactive, QPalette::Base, color);
    ui->editor->setPalette(palette);
    ui->editor->viewport()->setAutoFillBackground(true);
}

void MainWindow::toggleWordWrap(bool checked)
{
    ui->editor->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth
                                        : QPlainTextEdit::NoWrap);
}

// ==================== 视图操作 ====================

void MainWindow::toggleLineNumber(bool checked)
{
    ui->editor->setLineNumberVisible(checked);
}

void MainWindow::toggleToolBar(bool checked)
{
    ui->mainToolBar->setVisible(checked);
}

void MainWindow::toggleStatusBar(bool checked)
{
    ui->statusbar->setVisible(checked);
}

// ==================== 查找 / 替换 ====================

void MainWindow::showFindDialog()
{
    if (!m_findDialog) {
        m_findDialog = new FindDialog(this);
        connect(m_findDialog, &FindDialog::findNextRequested, this, &MainWindow::doFindNext);
    }

    // 打开对话框时，把编辑器里选中的文字直接带进查找框
    const QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection())
        m_findDialog->setFindText(cursor.selectedText());

    m_findDialog->setStatusText(QString());
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    if (!m_replaceDialog) {
        m_replaceDialog = new ReplaceDialog(this);
        connect(m_replaceDialog, &ReplaceDialog::findNextRequested, this, &MainWindow::doFindNext);
        connect(m_replaceDialog, &ReplaceDialog::replaceRequested, this, &MainWindow::doReplace);
        connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested, this, &MainWindow::doReplaceAll);
    }

    const QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection())
        m_replaceDialog->setFindText(cursor.selectedText());

    m_replaceDialog->setStatusText(QString());
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

bool MainWindow::findTextInEditor(const QString &text, bool caseSensitive, bool backward)
{
    if (text.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    if (backward)
        flags |= QTextDocument::FindBackward;

    bool found = ui->editor->find(text, flags);
    if (!found) {
        // 搜到文档末尾了，回到另一端再找一次，实现循环查找
        QTextCursor cursor = ui->editor->textCursor();
        cursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        ui->editor->setTextCursor(cursor);
        found = ui->editor->find(text, flags);
    }
    return found;
}

void MainWindow::doFindNext()
{
    // 可能是"查找"对话框或"替换"对话框发来的，按 sender() 区分
    FindDialog *findDlg = qobject_cast<FindDialog *>(sender());
    ReplaceDialog *replaceDlg = qobject_cast<ReplaceDialog *>(sender());

    const QString text = findDlg ? findDlg->findText()
                                 : (replaceDlg ? replaceDlg->findText() : QString());
    const bool caseSensitive = findDlg ? findDlg->caseSensitive()
                                       : (replaceDlg ? replaceDlg->caseSensitive() : false);
    const bool backward = findDlg ? findDlg->backward() : false;

    auto setStatus = [findDlg, replaceDlg](const QString &msg) {
        if (findDlg)
            findDlg->setStatusText(msg);
        else if (replaceDlg)
            replaceDlg->setStatusText(msg);
    };

    if (text.isEmpty()) {
        setStatus(QStringLiteral("请输入要查找的内容"));
        return;
    }

    if (findTextInEditor(text, caseSensitive, backward))
        setStatus(QString());
    else
        setStatus(QStringLiteral("未找到“%1”").arg(text));
}

void MainWindow::doReplace()
{
    if (!m_replaceDialog)
        return;

    const QString findText = m_replaceDialog->findText();
    const QString replaceText = m_replaceDialog->replaceText();
    if (findText.isEmpty()) {
        m_replaceDialog->setStatusText(QStringLiteral("请输入要查找的内容"));
        return;
    }

    // 当前选中的内容正好是要查找的文字，就先替换掉，再跳下一个
    QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == findText)
        cursor.insertText(replaceText);

    if (findTextInEditor(findText, m_replaceDialog->caseSensitive(), false))
        m_replaceDialog->setStatusText(QString());
    else
        m_replaceDialog->setStatusText(QStringLiteral("未找到“%1”").arg(findText));
}

void MainWindow::doReplaceAll()
{
    if (!m_replaceDialog)
        return;

    const QString findText = m_replaceDialog->findText();
    const QString replaceText = m_replaceDialog->replaceText();
    if (findText.isEmpty()) {
        m_replaceDialog->setStatusText(QStringLiteral("请输入要查找的内容"));
        return;
    }

    QTextDocument::FindFlags flags;
    if (m_replaceDialog->caseSensitive())
        flags |= QTextDocument::FindCaseSensitively;

    int count = 0;
    QTextCursor cursor(ui->editor->document());
    cursor.beginEditBlock();

    QTextCursor found = ui->editor->document()->find(findText, cursor, flags);
    while (!found.isNull() && count < 100000) {   // 上限保护，防止替换文本包含查找文本时死循环
        found.insertText(replaceText);
        ++count;
        found = ui->editor->document()->find(findText, found, flags);
    }

    cursor.endEditBlock();

    if (count == 0)
        m_replaceDialog->setStatusText(QStringLiteral("未找到“%1”").arg(findText));
    else
        m_replaceDialog->setStatusText(QStringLiteral("已替换 %1 处").arg(count));
}

// ==================== 其它 ====================

void MainWindow::showAbout()
{
    QMessageBox::about(
        this, QStringLiteral("关于"),
        QStringLiteral(
            "<h3>简单文本编辑器 v1.0</h3>"
            "<p>Qt 应用程序开发 · 实验二</p>"
            "<p>支持文件新建 / 打开 / 保存 / 另存为、剪切复制粘贴、撤销恢复、"
            "字体大小与颜色、背景色、自动换行、行号显示、查找与替换，"
            "并带 60 秒定时自动保存。</p>"
            "<p>编译环境：Qt %1 + MinGW</p>")
            .arg(QString::fromLatin1(qVersion())));
}

void MainWindow::onModificationChanged(bool changed)
{
    setWindowModified(changed);
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    if (!m_statusLabel)
        return;

    const QTextCursor cursor = ui->editor->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int column = cursor.columnNumber() + 1;
    const int chars = ui->editor->toPlainText().size();
    const QString state = ui->editor->document()->isModified()
        ? QStringLiteral("已修改")
        : QStringLiteral("已保存");

    m_statusLabel->setText(QStringLiteral("行 %1，列 %2    字符数 %3    %4")
                               .arg(line).arg(column).arg(chars).arg(state));
}

void MainWindow::updatePasteAction()
{
    ui->actionPaste->setEnabled(ui->editor->canPaste());
}

void MainWindow::autoSave()
{
    // 只有已经保存过（有文件路径）且确实有改动时才自动保存
    if (m_currentFile.isEmpty())
        return;
    if (!ui->editor->document()->isModified())
        return;

    if (saveToFile(m_currentFile)) {
        statusBar()->showMessage(
            QStringLiteral("已自动保存（%1）")
                .arg(QTime::currentTime().toString(QStringLiteral("hh:mm:ss"))),
            3000);
    }
}
