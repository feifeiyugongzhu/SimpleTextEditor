#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QLabel;

// 查找对话框（非模态，可以边看文本边查找）
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);

    QString findText() const;
    bool caseSensitive() const;
    bool backward() const;

    void setFindText(const QString &text);
    void setStatusText(const QString &text);

signals:
    void findNextRequested();

private:
    QLineEdit *m_lineEdit;
    QCheckBox *m_caseBox;
    QCheckBox *m_backwardBox;
    QLabel *m_statusLabel;
};

#endif // FINDDIALOG_H
