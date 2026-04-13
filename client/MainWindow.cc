#include "MainWindow.h"
#include "NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDebug>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_stackedWidget(new QStackedWidget(this))
    , m_listWidget(nullptr)
    , m_detailWidget(nullptr)
    , m_table(new QTableWidget(this))
    , m_refreshBtn(new QPushButton("刷新列表", this))
    , m_statusLabel(new QLabel("加载中...", this))
    , m_refreshTimer(new QTimer(this))
    , m_detailCountdownTimer(new QTimer(this))
    , m_detailCountdown(0)
    , m_detailNameLabel(nullptr)
    , m_detailPriceLabel(nullptr)
    , m_detailStockLabel(nullptr)
    , m_detailCountdownLabel(nullptr)
    , m_detailStatusLabel(nullptr)
    , m_detailStartTimeLabel(nullptr)
    , m_detailEndTimeLabel(nullptr)
    , m_detailTotalStockLabel(nullptr)
    , m_detailBuyBtn(nullptr)
    , m_backBtn(nullptr)
    , m_detailQuantitySpinBox(nullptr)
    , m_currentActivityId(0)
{
    setupUi();

    connect(&NetworkManager::instance(), &NetworkManager::activityListReceived,
            this, &MainWindow::onActivityListReceived);
    connect(&NetworkManager::instance(), &NetworkManager::countdownReceived,
            this, &MainWindow::onCountdownReceived);
    connect(&NetworkManager::instance(), &NetworkManager::stockReceived,
            this, &MainWindow::onStockReceived);
    connect(&NetworkManager::instance(), &NetworkManager::orderCreated,
            this, &MainWindow::onOrderCreated);
    connect(&NetworkManager::instance(), &NetworkManager::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClicked);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::loadActivities);
    connect(m_detailCountdownTimer, &QTimer::timeout, this, &MainWindow::onDetailCountdownTimerUpdate);

    m_refreshTimer->start(10000);
    loadActivities();
}

MainWindow::~MainWindow()
{
    m_detailCountdownTimer->stop();
}

void MainWindow::setupUi()
{
    setWindowTitle("秒杀活动列表");
    setMinimumSize(900, 600);

    // Create list widget
    m_listWidget = new QWidget(this);
    QVBoxLayout* listLayout = new QVBoxLayout(m_listWidget);

    QLabel* title = new QLabel("秒杀活动列表", m_listWidget);
    title->setStyleSheet("font-size: 24px; font-weight: bold;");
    listLayout->addWidget(title);

    listLayout->addWidget(m_statusLabel);

    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"ID", "活动名称", "价格", "总库存", "剩余库存", "状态"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    connect(m_table, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onActivityDoubleClicked);
    listLayout->addWidget(m_table);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addStretch();
    listLayout->addLayout(btnLayout);

    m_stackedWidget->addWidget(m_listWidget);
    setCentralWidget(m_stackedWidget);
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
            QString endTimeStr = activity["end_time"].toString();
            QDateTime endTime = QDateTime::fromString(endTimeStr, "yyyy-MM-dd HH:mm:ss");
            QDateTime now = QDateTime::currentDateTime();
            if (endTime.msecsTo(now) > 3600 * 1000) {
                continue;
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
    if (m_stackedWidget->currentWidget() == m_detailWidget) {
        QMessageBox::critical(this, "错误", error);
    }
    m_statusLabel->setText(QString("错误: %1").arg(error));
}

void MainWindow::onActivityDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    if (row >= 0 && row < m_activities.size()) {
        QJsonObject activity = m_activities[row].toObject();
        showActivityDetail(activity);
    }
}

void MainWindow::showActivityDetail(const QJsonObject& activity)
{
    m_currentActivity = activity;
    m_currentActivityId = activity["id"].toInt();
    m_detailCountdown = 0;
    m_detailStatus = QString::number(activity["status"].toInt());

    // Create detail widget if not exists
    if (!m_detailWidget) {
        m_detailWidget = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(m_detailWidget);

        // Back button
        m_backBtn = new QPushButton("返回列表", m_detailWidget);
        connect(m_backBtn, &QPushButton::clicked, this, &MainWindow::onBackToList);
        layout->addWidget(m_backBtn);

        // Title
        m_detailNameLabel = new QLabel(m_detailWidget);
        m_detailNameLabel->setStyleSheet("font-size: 28px; font-weight: bold;");
        layout->addWidget(m_detailNameLabel);

        // Info group
        QGroupBox* infoGroup = new QGroupBox("活动信息", m_detailWidget);
        QFormLayout* infoLayout = new QFormLayout(infoGroup);
        m_detailPriceLabel = new QLabel(infoGroup);
        m_detailStartTimeLabel = new QLabel(infoGroup);
        m_detailEndTimeLabel = new QLabel(infoGroup);
        m_detailTotalStockLabel = new QLabel(infoGroup);
        m_detailStockLabel = new QLabel(infoGroup);
        infoLayout->addRow("秒杀价格:", m_detailPriceLabel);
        infoLayout->addRow("开始时间:", m_detailStartTimeLabel);
        infoLayout->addRow("结束时间:", m_detailEndTimeLabel);
        infoLayout->addRow("总库存:", m_detailTotalStockLabel);
        infoLayout->addRow("剩余库存:", m_detailStockLabel);
        layout->addWidget(infoGroup);

        // Status group
        QGroupBox* statusGroup = new QGroupBox("实时状态", m_detailWidget);
        QHBoxLayout* statusLayout = new QHBoxLayout(statusGroup);

        QVBoxLayout* stockLayout = new QVBoxLayout();
        stockLayout->addWidget(new QLabel("剩余库存:"));
        m_detailStockLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #e74c3c;");
        stockLayout->addWidget(m_detailStockLabel);
        statusLayout->addLayout(stockLayout);

        QVBoxLayout* countdownLayout = new QVBoxLayout();
        countdownLayout->addWidget(new QLabel("倒计时:"));
        m_detailCountdownLabel = new QLabel("加载中...", m_detailWidget);
        m_detailCountdownLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #3498db;");
        countdownLayout->addWidget(m_detailCountdownLabel);
        statusLayout->addLayout(countdownLayout);

        m_detailStatusLabel = new QLabel(m_detailWidget);
        statusLayout->addWidget(m_detailStatusLabel);

        layout->addWidget(statusGroup);

        // Purchase group
        QGroupBox* purchaseGroup = new QGroupBox("购买", m_detailWidget);
        QVBoxLayout* purchaseLayout = new QVBoxLayout(purchaseGroup);

        QHBoxLayout* quantityLayout = new QHBoxLayout();
        quantityLayout->addWidget(new QLabel("购买数量:"));
        m_detailQuantitySpinBox = new QSpinBox(m_detailWidget);
        m_detailQuantitySpinBox->setMinimum(1);
        m_detailQuantitySpinBox->setMaximum(10);
        m_detailQuantitySpinBox->setValue(1);
        quantityLayout->addWidget(m_detailQuantitySpinBox);
        quantityLayout->addStretch();
        purchaseLayout->addLayout(quantityLayout);

        m_detailBuyBtn = new QPushButton("立即抢购", m_detailWidget);
        m_detailBuyBtn->setMinimumHeight(50);
        m_detailBuyBtn->setStyleSheet("background-color: #e74c3c; color: white; font-size: 18px; font-weight: bold;");
        connect(m_detailBuyBtn, &QPushButton::clicked, this, &MainWindow::onDetailBuyClicked);
        purchaseLayout->addWidget(m_detailBuyBtn);

        layout->addWidget(purchaseGroup);
        layout->addStretch();

        m_stackedWidget->addWidget(m_detailWidget);
    }

    // Update detail info
    m_detailNameLabel->setText(activity["name"].toString());
    m_detailPriceLabel->setText(QString("¥%1").arg(activity["price"].toDouble()));
    m_detailStartTimeLabel->setText(activity["start_time"].toString());
    m_detailEndTimeLabel->setText(activity["end_time"].toString());
    m_detailTotalStockLabel->setText(QString::number(activity["total_stock"].toInt()));
    m_detailStockLabel->setText(QString::number(activity["remain_stock"].toInt()));
    m_detailCountdownLabel->setText("加载中...");

    // Fetch real-time data
    int activityId = activity["id"].toInt();
    NetworkManager::instance().getCountdown(activityId);
    NetworkManager::instance().getActivityStock(activityId);

    // Switch to detail view
    m_stackedWidget->setCurrentWidget(m_detailWidget);
    updateDetailDisplay();
}

void MainWindow::showActivityList()
{
    m_detailCountdownTimer->stop();
    m_stackedWidget->setCurrentWidget(m_listWidget);
}

void MainWindow::onBackToList()
{
    showActivityList();
}

void MainWindow::onDetailBuyClicked()
{
    m_detailBuyBtn->setEnabled(false);

    if (m_detailStatus == "2") {
        QMessageBox::warning(this, "提示", "活动已结束！");
        m_detailBuyBtn->setText("已结束");
        return;
    }

    if (m_detailStatus == "0") {
        QMessageBox::warning(this, "提示", "活动尚未开始，请等待倒计时结束！");
        m_detailBuyBtn->setText("等待开始");
        return;
    }

    int stock = m_detailStockLabel->text().toInt();
    if (stock <= 0) {
        QMessageBox::warning(this, "提示", "库存不足，已售罄！");
        m_detailBuyBtn->setText("已售罄");
        return;
    }

    int activityId = m_currentActivity["id"].toInt();
    int userId = NetworkManager::instance().getUserId();
    int quantity = m_detailQuantitySpinBox->value();

    m_detailBuyBtn->setText("抢购中...");

    NetworkManager::instance().createSeckillOrder(activityId, userId, quantity);
}

void MainWindow::onCountdownReceived(int countdown, const QString& status)
{
    if (m_stackedWidget->currentWidget() != m_detailWidget) return;

    m_detailCountdown = countdown;
    m_detailStatus = status;
    updateDetailDisplay();

    if (status == "2") {
        m_detailCountdownTimer->stop();
    }
}

void MainWindow::onStockReceived(int remainStock)
{
    if (m_stackedWidget->currentWidget() != m_detailWidget) return;

    m_detailStockLabel->setText(QString::number(remainStock));
    if (remainStock <= 0) {
        m_detailStockLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #95a5a6;");
        m_detailBuyBtn->setEnabled(false);
        m_detailBuyBtn->setText("已售罄");
    } else if (m_detailStatus == "1") {
        m_detailBuyBtn->setEnabled(true);
        m_detailBuyBtn->setText("立即抢购");
    }
}

void MainWindow::onOrderCreated(const QJsonObject& result)
{
    QString orderNo = result["order_no"].toString();
    QString message = result["message"].toString();

    QMessageBox::information(this, "抢购成功",
        QString("订单号: %1\n状态: %2").arg(orderNo).arg(message));
    m_detailBuyBtn->setText("立即抢购");
    m_detailBuyBtn->setEnabled(true);

    // Refresh stock
    NetworkManager::instance().getActivityStock(m_currentActivity["id"].toInt());
}

void MainWindow::onDetailCountdownTimerUpdate()
{
    if (m_detailCountdown > 0) {
        m_detailCountdown--;
        updateDetailDisplay();
    } else if (m_detailStatus == "1") {
        // Activity is active, re-fetch countdown to check if it ends
        NetworkManager::instance().getCountdown(m_currentActivityId);
    }
}

void MainWindow::updateDetailDisplay()
{
    if (m_detailStatus == "2") {
        m_detailCountdownLabel->setText("活动已结束");
        m_detailCountdownLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #95a5a6;");
        m_detailBuyBtn->setEnabled(false);
        m_detailBuyBtn->setText("已结束");
        m_detailCountdownTimer->stop();
    } else if (m_detailStatus == "1") {
        m_detailCountdownLabel->setText("进行中");
        m_detailCountdownLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #27ae60;");
        int stock = m_detailStockLabel->text().toInt();
        if (stock > 0) {
            m_detailBuyBtn->setEnabled(true);
            m_detailBuyBtn->setText("立即抢购");
        } else {
            m_detailBuyBtn->setEnabled(false);
            m_detailBuyBtn->setText("已售罄");
        }
        m_detailCountdownTimer->start(1000);
    } else {
        int hours = m_detailCountdown / 3600;
        int minutes = (m_detailCountdown % 3600) / 60;
        int seconds = m_detailCountdown % 60;
        m_detailCountdownLabel->setText(QString("%1:%2:%3")
                                   .arg(hours, 2, 10, QChar('0'))
                                   .arg(minutes, 2, 10, QChar('0'))
                                   .arg(seconds, 2, 10, QChar('0')));
        m_detailCountdownLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #3498db;");
        m_detailBuyBtn->setEnabled(false);
        m_detailBuyBtn->setText("等待开始");
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
