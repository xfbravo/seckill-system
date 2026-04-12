#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QJsonArray>

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

private:
    void setupUi();
    void loadActivities();
    QString formatCountdown(int seconds);
    QString getStatusText(int status);

private:
    QTableWidget* m_table;
    QPushButton* m_refreshBtn;
    QLabel* m_statusLabel;
    QTimer* m_refreshTimer;
    QJsonArray m_activities;
};

#endif // MAINWINDOW_H
