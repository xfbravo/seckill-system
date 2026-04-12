#include <QApplication>
#include <QStyleFactory>
#include <QNetworkProxyFactory>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("秒杀客户端");
    app.setApplicationVersion("1.0.0");
    app.setStyle(QStyleFactory::create("Fusion"));

    // Set network proxy to use system settings
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    MainWindow w;
    w.show();

    return app.exec();
}
