#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    int getUserId() const { return m_userId; }
    bool isLoggedIn() const { return m_loggedIn; }

private slots:
    void onLoginClicked();

private:
    void performLogin();

private:
    QLineEdit* m_userIdInput;
    QPushButton* m_loginBtn;
    QLabel* m_statusLabel;
    int m_userId;
    bool m_loggedIn;
};

#endif // LOGINWINDOW_H
