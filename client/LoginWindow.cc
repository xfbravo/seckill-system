#include "LoginWindow.h"
#include "NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
    , m_userIdInput(new QLineEdit(this))
    , m_loginBtn(new QPushButton("登录", this))
    , m_statusLabel(new QLabel("", this))
    , m_userId(0)
    , m_loggedIn(false)
{
    setWindowTitle("用户登录");
    setMinimumWidth(300);
    setModal(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* title = new QLabel("请输入用户ID", this);
    title->setStyleSheet("font-size: 16px; font-weight: bold;");
    mainLayout->addWidget(title);

    m_userIdInput->setPlaceholderText("用户ID (如: 1001)");
    m_userIdInput->setMaxLength(20);
    mainLayout->addWidget(m_userIdInput);

    mainLayout->addWidget(m_loginBtn);

    m_statusLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(m_statusLabel);

    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_userIdInput, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::onLoginClicked()
{
    QString idStr = m_userIdInput->text().trimmed();
    if (idStr.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户ID");
        return;
    }

    bool ok;
    int userId = idStr.toInt(&ok);
    if (!ok || userId <= 0) {
        QMessageBox::warning(this, "提示", "用户ID必须是正整数");
        return;
    }

    m_userId = userId;
    performLogin();
}

void LoginWindow::performLogin()
{
    m_loginBtn->setEnabled(false);
    m_statusLabel->setText("正在登录...");
    m_statusLabel->setStyleSheet("color: blue;");

    // 直接使用API登录
    // 这里简化处理：假设登录成功
    // 实际项目中应该调用API验证
    m_loggedIn = true;
    m_statusLabel->setText("登录成功！");
    m_statusLabel->setStyleSheet("color: green;");

    accept();
}
