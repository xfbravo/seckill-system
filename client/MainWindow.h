#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QJsonArray>
#include <QStackedWidget>
#include <QWidget>
#include <QJsonObject>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRefreshClicked();
    void onActivityDoubleClicked(int row, int column);
    void onActivityListReceived(const QJsonArray& list);
    void onErrorOccurred(const QString& error);
    void onBackToList();

private:
    void setupUi();
    void loadActivities();
    void showActivityDetail(const QJsonObject& activity);
    void showActivityList();
    QString formatCountdown(int seconds);
    QString getStatusText(int status);

private:
    QStackedWidget* m_stackedWidget;
    QWidget* m_listWidget;
    QWidget* m_detailWidget;
    QTableWidget* m_table;
    QPushButton* m_refreshBtn;
    QLabel* m_statusLabel;
    QTimer* m_refreshTimer;
    QJsonArray m_activities;

    // Detail view widgets
    QLabel* m_detailNameLabel;
    QLabel* m_detailPriceLabel;
    QLabel* m_detailStockLabel;
    QLabel* m_detailCountdownLabel;
    QLabel* m_detailStatusLabel;
    QPushButton* m_detailBuyBtn;
    QPushButton* m_backBtn;
    QSpinBox* m_detailQuantitySpinBox;
    QJsonObject m_currentActivity;
    int m_currentActivityId;
    QTimer* m_detailCountdownTimer;
    int m_detailCountdown;
    QString m_detailStatus;

    // Detail view slots
    void onDetailBuyClicked();
    void onCountdownReceived(int countdown, const QString& status);
    void onStockReceived(int remainStock);
    void onOrderCreated(const QJsonObject& result);
    void onDetailCountdownTimerUpdate();
    void updateDetailDisplay();
};

#endif // MAINWINDOW_H
