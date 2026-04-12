#include "NetworkManager.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_serverUrl("http://localhost:8080")
    , m_currentReply(nullptr)
{
}

NetworkManager& NetworkManager::instance()
{
    static NetworkManager instance;
    return instance;
}

void NetworkManager::setServerUrl(const QString& url)
{
    m_serverUrl = url;
}

void NetworkManager::get(const QString& path)
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }

    QUrl url(m_serverUrl + path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_currentReply = m_nam->get(request);

    connect(m_currentReply, &QNetworkReply::finished, this, &NetworkManager::onGetFinished);
}

void NetworkManager::post(const QString& path, const QJsonObject& body)
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }

    QUrl url(m_serverUrl + path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data = QJsonDocument(body).toJson();
    m_currentReply = m_nam->post(request, data);

    connect(m_currentReply, &QNetworkReply::finished, this, &NetworkManager::onPostFinished);
}

void NetworkManager::getActivityList()
{
    get("/api/activity/list");
}

void NetworkManager::getActivity(int id)
{
    get(QString("/api/activity/%1").arg(id));
}

void NetworkManager::getActivityStock(int id)
{
    get(QString("/api/activity/%1/stock").arg(id));
}

void NetworkManager::getCountdown(int id)
{
    get(QString("/api/seckill/countdown/%1").arg(id));
}

void NetworkManager::createSeckillOrder(int activityId, int userId, int quantity)
{
    QJsonObject body;
    body["activity_id"] = activityId;
    body["user_id"] = userId;
    body["quantity"] = quantity;
    post("/api/seckill/order", body);
}

void NetworkManager::getOrder(int orderId)
{
    get(QString("/api/order/%1").arg(orderId));
}

void NetworkManager::getOrderStatus(int orderId)
{
    get(QString("/api/order/status/%1").arg(orderId));
}

void NetworkManager::onGetFinished()
{
    if (!m_currentReply) return;

    if (m_currentReply->error() != QNetworkReply::NoError) {
        emit errorOccurred(m_currentReply->errorString());
        return;
    }

    QByteArray data = m_currentReply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject json = doc.object();

    int code = json["code"].toInt();
    if (code != 0) {
        emit errorOccurred(json["message"].toString());
        return;
    }

    QString path = m_currentReply->url().path();

    if (path.contains("/api/activity/list")) {
        emit activityListReceived(json["data"].toArray());
    } else if (path.contains("/api/activity/") && path.contains("/stock")) {
        QJsonObject dataObj = json["data"].toObject();
        emit stockReceived(dataObj["remain_stock"].toInt());
    } else if (path.contains("/api/seckill/countdown")) {
        QJsonObject dataObj = json["data"].toObject();
        emit countdownReceived(dataObj["countdown"].toInt(), dataObj["status"].toString());
    } else if (path.contains("/api/activity/")) {
        emit activityReceived(json["data"].toObject());
    } else if (path.contains("/api/order/status")) {
        QJsonObject dataObj = json["data"].toObject();
        emit orderStatusReceived(dataObj["status"].toString(),
                                dataObj["status_text"].toString());
    } else if (path.contains("/api/order/")) {
        emit orderReceived(json["data"].toObject());
    }
}

void NetworkManager::onPostFinished()
{
    if (!m_currentReply) return;

    if (m_currentReply->error() != QNetworkReply::NoError) {
        emit errorOccurred(m_currentReply->errorString());
        return;
    }

    QByteArray data = m_currentReply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject json = doc.object();

    int code = json["code"].toInt();
    if (code != 0) {
        emit errorOccurred(json["message"].toString());
        return;
    }

    QString path = m_currentReply->url().path();

    if (path.contains("/api/seckill/order")) {
        emit orderCreated(json["data"].toObject());
    }
}
