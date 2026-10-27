#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QLabel;

// 替换对话框（非模态）
class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);

    QString findText() const;
    QString replaceText() const;
    bool caseSensitive() const;

    void setFindText(const QString &text);
    void setStatusText(const QString &text);

signals:
    void findNextRequested();
    void replaceRequested();
    void replaceAllRequested();

private:
    QLineEdit *m_findEdit;
    QLineEdit *m_replaceEdit;
    QCheckBox *m_caseBox;
    QLabel *m_statusLabel;
};

#endif // REPLACEDIALOG_H
