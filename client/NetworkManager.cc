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

void NetworkManager::setUserId(int userId)
{
    QSettings settings("SeckillApp", "Client");
    settings.setValue("userId", QString::number(userId));
}

int NetworkManager::getUserId()
{
    QSettings settings("SeckillApp", "Client");
    QString userIdStr = settings.value("userId").toString();
    if (userIdStr.isEmpty()) {
        return 0;
    }
    return userIdStr.toInt();
}

void NetworkManager::get(const QString& path)
{
    QUrl url(m_serverUrl + path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, &NetworkManager::onGetFinished);
}

void NetworkManager::post(const QString& path, const QJsonObject& body)
{
    if (m_currentReply) {
        m_currentReply->abort();
        disconnect(m_currentReply, &QNetworkReply::finished, this, &NetworkManager::onPostFinished);
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
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

void NetworkManager::login(int userId)
{
    QJsonObject body;
    body["user_id"] = userId;
    post("/api/user/login", body);
}

bool NetworkManager::loginOrCreate(int userId)
{
    // Simplified: just set the user ID locally without API call
    QSettings settings("SeckillApp", "Client");
    settings.setValue("userId", QString::number(userId));
    return true;
}

void NetworkManager::onGetFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    // Ignore OperationCanceledError - this happens when we abort a request to start a new one
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        return;
    }

    qDebug() << "onGetFinished success, url:" << reply->url().path();

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject json = doc.object();

    int code = json["code"].toInt();
    if (code != 0) {
        emit errorOccurred(json["message"].toString());
        return;
    }

    QString path = reply->url().path();

    if (path.contains("/api/activity/list")) {
        emit activityListReceived(json["data"].toArray());
    } else if (path.contains("/api/activity/") && path.contains("/stock")) {
        QJsonObject dataObj = json["data"].toObject();
        emit stockReceived(dataObj["remain_stock"].toInt());
} else if (path.contains("/api/seckill/countdown")) {
        QJsonObject dataObj = json["data"].toObject();
        emit countdownReceived(dataObj["countdown"].toInt(), QString::number(dataObj["status"].toInt()));
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
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    // Ignore OperationCanceledError - this happens when we abort a request to start a new one
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject json = doc.object();

    int code = json["code"].toInt();
    QString path = reply->url().path();

    if (code != 0) {
        if (path.contains("/api/user/login")) {
            emit loginFailed(json["message"].toString());
        } else {
            emit errorOccurred(json["message"].toString());
        }
        return;
    }

    if (path.contains("/api/seckill/order")) {
        emit orderCreated(json["data"].toObject());
    } else if (path.contains("/api/user/login")) {
        QJsonObject dataObj = json["data"].toObject();
        int userId = dataObj["id"].toInt();
        emit loginSuccess(userId);
    }
}
