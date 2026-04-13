#include "MainWindow.h"
#include "ActivityDetailWindow.h"
#include "NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_table(new QTableWidget(this))
    , m_refreshBtn(new QPushButton("刷新列表", this))
    , m_statusLabel(new QLabel("加载中...", this))
    , m_refreshTimer(new QTimer(this))
{
    setupUi();

    connect(&NetworkManager::instance(), &NetworkManager::activityListReceived,
            this, &MainWindow::onActivityListReceived);
    connect(&NetworkManager::instance(), &NetworkManager::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::loadActivities);

    m_refreshTimer->start(10000); // Auto refresh every 10 seconds
    loadActivities();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    setWindowTitle("秒杀活动列表");
    setMinimumSize(900, 600);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Header
    QLabel* title = new QLabel("秒杀活动列表", this);
    title->setStyleSheet("font-size: 24px; font-weight: bold;");
    mainLayout->addWidget(title);

    // Status label
    mainLayout->addWidget(m_statusLabel);

    // Table
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"ID", "活动名称", "价格", "总库存", "剩余库存", "状态"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    connect(m_table, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onActivityDoubleClicked);
    mainLayout->addWidget(m_table);

    // Button
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::loadActivities()
{
    m_statusLabel->setText("加载中...");
    NetworkManager::instance().getActivityList();
}

void MainWindow::onRefreshClicked()
{
    loadActivities();
}

void MainWindow::onActivityListReceived(const QJsonArray& list)
{
    // Filter: hide activities ended more than 1 hour ago
    QJsonArray filteredList;
    for (int i = 0; i < list.size(); ++i) {
        QJsonObject activity = list[i].toObject();
        int status = activity["status"].toInt();

        if (status == 2) {
            // Activity ended, check if more than 1 hour ago
            QString endTimeStr = activity["end_time"].toString();
            QDateTime endTime = QDateTime::fromString(endTimeStr, "yyyy-MM-dd HH:mm:ss");
            QDateTime now = QDateTime::currentDateTime();
            if (endTime.msecsTo(now) > 3600 * 1000) {
                continue; // Skip this activity
            }
        }
        filteredList.append(activity);
    }

    m_activities = filteredList;
    m_table->setRowCount(filteredList.size());

    for (int i = 0; i < filteredList.size(); ++i) {
        QJsonObject activity = filteredList[i].toObject();

        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(activity["id"].toInt())));
        m_table->setItem(i, 1, new QTableWidgetItem(activity["name"].toString()));
        m_table->setItem(i, 2, new QTableWidgetItem(QString("¥%1").arg(activity["price"].toDouble())));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(activity["total_stock"].toInt())));
        m_table->setItem(i, 4, new QTableWidgetItem(QString::number(activity["remain_stock"].toInt())));

        int status = activity["status"].toInt();
        QTableWidgetItem* statusItem = new QTableWidgetItem(getStatusText(status));
        statusItem->setForeground(QBrush(status == 1 ? Qt::green : (status == 2 ? Qt::red : Qt::gray)));
        m_table->setItem(i, 5, statusItem);
    }

    m_statusLabel->setText(QString("共 %1 个活动，最后更新: %2")
                           .arg(filteredList.size())
                           .arg(QTime::currentTime().toString("HH:mm:ss")));
}

void MainWindow::onErrorOccurred(const QString& error)
{
    m_statusLabel->setText(QString("错误: %1").arg(error));
}

void MainWindow::onActivityDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0 && row < m_activities.size()) {
        QJsonObject activity = m_activities[row].toObject();
        ActivityDetailWindow* detailWindow = new ActivityDetailWindow(activity, this);
        detailWindow->setAttribute(Qt::WA_DeleteOnClose);
        detailWindow->show();
    }
}

QString MainWindow::formatCountdown(int seconds)
{
    if (seconds <= 0) return "已开始";

    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0'))
                               .arg(minutes, 2, 10, QChar('0'))
                               .arg(secs, 2, 10, QChar('0'));
}

QString MainWindow::getStatusText(int status)
{
    switch (status) {
        case 0: return "未开始";
        case 1: return "进行中";
        case 2: return "已结束";
        default: return "未知";
    }
}
