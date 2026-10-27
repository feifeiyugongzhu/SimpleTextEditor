#include "finddialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("查找"));
    // 非模态：打开后仍然可以操作主窗口里的文本
    setModal(false);

    m_lineEdit = new QLineEdit(this);
    m_caseBox = new QCheckBox(QStringLiteral("区分大小写"), this);
    m_backwardBox = new QCheckBox(QStringLiteral("向上查找"), this);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(QStringLiteral("color: #c0392b;"));

    QPushButton *btnFind = new QPushButton(QStringLiteral("查找下一个"), this);
    btnFind->setDefault(true);
    QPushButton *btnClose = new QPushButton(QStringLiteral("关闭"), this);

    QGridLayout *form = new QGridLayout;
    form->addWidget(new QLabel(QStringLiteral("查找内容:"), this), 0, 0);
    form->addWidget(m_lineEdit, 0, 1);
    form->addWidget(m_caseBox, 1, 0);
    form->addWidget(m_backwardBox, 1, 1);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(m_statusLabel, 1);
    buttons->addWidget(btnFind);
    buttons->addWidget(btnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addLayout(buttons);

    // 点按钮 / 在输入框里按回车，都触发查找
    connect(btnFind, &QPushButton::clicked, this, &FindDialog::findNextRequested);
    connect(m_lineEdit, &QLineEdit::returnPressed, this, &FindDialog::findNextRequested);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::close);

    resize(380, 140);
}

QString FindDialog::findText() const
{
    return m_lineEdit->text();
}

bool FindDialog::caseSensitive() const
{
    return m_caseBox->isChecked();
}

bool FindDialog::backward() const
{
    return m_backwardBox->isChecked();
}

void FindDialog::setFindText(const QString &text)
{
    m_lineEdit->setText(text);
    m_lineEdit->selectAll();
}

void FindDialog::setStatusText(const QString &text)
{
    m_statusLabel->setText(text);
}
