#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QUuid>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static NetworkManager& instance();

    // API calls
    void getActivityList();
    void getActivity(int id);
    void getActivityStock(int id);
    void getCountdown(int id);
    void createSeckillOrder(int activityId, int userId, int quantity = 1);
    void getOrder(int orderId);
    void getOrderStatus(int orderId);

    // User identification
    QString getOrCreateUserId();
    int getUserId();

    // Server configuration
    void setServerUrl(const QString& url);

signals:
    void activityListReceived(const QJsonArray& list);
    void activityReceived(const QJsonObject& activity);
    void stockReceived(int remainStock);
    void countdownReceived(int countdown, const QString& status);
    void orderCreated(const QJsonObject& result);
    void orderReceived(const QJsonObject& order);
    void orderStatusReceived(const QString& status, const QString& statusText);
    void errorOccurred(const QString& error);

private:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager() = default;
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    void get(const QString& path);
    void post(const QString& path, const QJsonObject& body);

private slots:
    void onGetFinished();
    void onPostFinished();

private:
    QNetworkAccessManager* m_nam;
    QString m_serverUrl;
    QNetworkReply* m_currentReply;
};

#endif // NETWORKMANAGER_H
