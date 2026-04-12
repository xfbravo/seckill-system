#ifndef ACTIVITYDETAILWINDOW_H
#define ACTIVITYDETAILWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QSpinBox>
#include <QJsonObject>

class ActivityDetailWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ActivityDetailWindow(const QJsonObject& activity, QWidget *parent = nullptr);
    ~ActivityDetailWindow();

private slots:
    void onCountdownReceived(int countdown, const QString& status);
    void onStockReceived(int remainStock);
    void onCountdownTimerUpdate();
    void onBuyClicked();
    void onOrderCreated(const QJsonObject& result);
    void onOrderStatusReceived(const QString& status, const QString& statusText);
    void onErrorOccurred(const QString& error);

private:
    void setupUi();
    void updateCountdownDisplay();
    void updateStockDisplay();

private:
    QJsonObject m_activity;
    QLabel* m_countdownLabel;
    QLabel* m_stockLabel;
    QPushButton* m_buyBtn;
    QSpinBox* m_quantitySpinBox;
    QTimer* m_countdownTimer;
    int m_countdown;
    int m_remainStock;
    QString m_status;
};

#endif // ACTIVITYDETAILWINDOW_H
