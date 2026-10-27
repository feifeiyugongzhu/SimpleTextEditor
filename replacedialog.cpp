#include "replacedialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

ReplaceDialog::ReplaceDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("替换"));
    setModal(false);

    m_findEdit = new QLineEdit(this);
    m_replaceEdit = new QLineEdit(this);
    m_caseBox = new QCheckBox(QStringLiteral("区分大小写"), this);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(QStringLiteral("color: #c0392b;"));

    QPushButton *btnFind = new QPushButton(QStringLiteral("查找下一个"), this);
    QPushButton *btnReplace = new QPushButton(QStringLiteral("替换"), this);
    btnReplace->setDefault(true);
    QPushButton *btnReplaceAll = new QPushButton(QStringLiteral("全部替换"), this);
    QPushButton *btnClose = new QPushButton(QStringLiteral("关闭"), this);

    QGridLayout *form = new QGridLayout;
    form->addWidget(new QLabel(QStringLiteral("查找内容:"), this), 0, 0);
    form->addWidget(m_findEdit, 0, 1);
    form->addWidget(new QLabel(QStringLiteral("替换为:"), this), 1, 0);
    form->addWidget(m_replaceEdit, 1, 1);
    form->addWidget(m_caseBox, 2, 0);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(m_statusLabel, 1);
    buttons->addWidget(btnFind);
    buttons->addWidget(btnReplace);
    buttons->addWidget(btnReplaceAll);
    buttons->addWidget(btnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addLayout(buttons);

    connect(btnFind, &QPushButton::clicked, this, &ReplaceDialog::findNextRequested);
    connect(btnReplace, &QPushButton::clicked, this, &ReplaceDialog::replaceRequested);
    connect(btnReplaceAll, &QPushButton::clicked, this, &ReplaceDialog::replaceAllRequested);
    connect(m_findEdit, &QLineEdit::returnPressed, this, &ReplaceDialog::findNextRequested);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::close);

    resize(460, 170);
}

QString ReplaceDialog::findText() const
{
    return m_findEdit->text();
}

QString ReplaceDialog::replaceText() const
{
    return m_replaceEdit->text();
}

bool ReplaceDialog::caseSensitive() const
{
    return m_caseBox->isChecked();
}

void ReplaceDialog::setFindText(const QString &text)
{
    m_findEdit->setText(text);
    m_findEdit->selectAll();
}

void ReplaceDialog::setStatusText(const QString &text)
{
    m_statusLabel->setText(text);
}
