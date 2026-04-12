#include <QApplication>
#include <QStyleFactory>
#include <QNetworkProxyFactory>
#include <QSettings>
#include "MainWindow.h"
#include "LoginWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("秒杀客户端");
    app.setApplicationVersion("1.0.0");
    app.setStyle(QStyleFactory::create("Fusion"));

    // Set network proxy to use system settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    // Always show login window
    QSettings settings("SeckillApp", "Client");
    int userId = 0;
    LoginWindow login;
    if (login.exec() == QDialog::Accepted) {
        userId = login.getUserId();
        settings.setValue("userId", QString::number(userId));
    } else {
        return 0;
    }

    MainWindow w;
    w.show();

    return app.exec();
}
