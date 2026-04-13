#include "ActivityDetailWindow.h"
#include "NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QDebug>

ActivityDetailWindow::ActivityDetailWindow(const QJsonObject& activity, QWidget *parent)
    : QMainWindow(parent)
    , m_activity(activity)
    , m_countdownLabel(new QLabel("加载中...", this))
    , m_stockLabel(new QLabel("加载中...", this))
    , m_buyBtn(new QPushButton("立即抢购", this))
    , m_quantitySpinBox(new QSpinBox(this))
    , m_countdownTimer(new QTimer(this))
    , m_countdown(0)
    , m_remainStock(0)
    , m_status("0")
{
    setWindowTitle(activity["name"].toString());
    setMinimumSize(500, 400);

    setupUi();

    qDebug() << "Connecting NetworkManager signals...";
    connect(&NetworkManager::instance(), &NetworkManager::countdownReceived,
            this, &ActivityDetailWindow::onCountdownReceived);
    connect(&NetworkManager::instance(), &NetworkManager::stockReceived,
            this, &ActivityDetailWindow::onStockReceived);
    connect(&NetworkManager::instance(), &NetworkManager::orderCreated,
            this, &ActivityDetailWindow::onOrderCreated);
    connect(&NetworkManager::instance(), &NetworkManager::orderStatusReceived,
            this, &ActivityDetailWindow::onOrderStatusReceived);
    connect(&NetworkManager::instance(), &NetworkManager::errorOccurred,
            this, &ActivityDetailWindow::onErrorOccurred);

    connect(m_buyBtn, &QPushButton::clicked, this, &ActivityDetailWindow::onBuyClicked);
    connect(m_countdownTimer, &QTimer::timeout, this, &ActivityDetailWindow::onCountdownTimerUpdate);

    // Load initial data
    int activityId = activity["id"].toInt();
    qDebug() << "Fetching countdown for activity:" << activityId;
    NetworkManager::instance().getCountdown(activityId);
    qDebug() << "Fetching stock for activity:" << activityId;
    NetworkManager::instance().getActivityStock(activityId);

    m_countdownTimer->start(1000); // Update every second
}

ActivityDetailWindow::~ActivityDetailWindow()
{
    m_countdownTimer->stop();
}

void ActivityDetailWindow::setupUi()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Title
    QLabel* title = new QLabel(m_activity["name"].toString(), this);
    title->setStyleSheet("font-size: 28px; font-weight: bold;");
    mainLayout->addWidget(title);

    // Activity info group
    QGroupBox* infoGroup = new QGroupBox("活动信息", this);
    QFormLayout* infoLayout = new QFormLayout(infoGroup);

    infoLayout->addRow("活动ID:", new QLabel(QString::number(m_activity["id"].toInt())));
    infoLayout->addRow("秒杀价格:", new QLabel(QString("¥%1").arg(m_activity["price"].toDouble())));
    infoLayout->addRow("开始时间:", new QLabel(m_activity["start_time"].toString()));
    infoLayout->addRow("结束时间:", new QLabel(m_activity["end_time"].toString()));
    infoLayout->addRow("总库存:", new QLabel(QString::number(m_activity["total_stock"].toInt())));

    mainLayout->addWidget(infoGroup);

    // Stock and countdown group
    QGroupBox* statusGroup = new QGroupBox("实时状态", this);
    QHBoxLayout* statusLayout = new QHBoxLayout(statusGroup);

    QVBoxLayout* stockLayout = new QVBoxLayout();
    stockLayout->addWidget(new QLabel("剩余库存:"));
    m_stockLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #e74c3c;");
    stockLayout->addWidget(m_stockLabel);
    statusLayout->addLayout(stockLayout);

    QVBoxLayout* countdownLayout = new QVBoxLayout();
    countdownLayout->addWidget(new QLabel("倒计时:"));
    m_countdownLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #3498db;");
    countdownLayout->addWidget(m_countdownLabel);
    statusLayout->addLayout(countdownLayout);

    mainLayout->addWidget(statusGroup);

    // Purchase group
    QGroupBox* purchaseGroup = new QGroupBox("购买", this);
    QVBoxLayout* purchaseLayout = new QVBoxLayout(purchaseGroup);

    QHBoxLayout* quantityLayout = new QHBoxLayout();
    quantityLayout->addWidget(new QLabel("购买数量:"));
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(10);
    m_quantitySpinBox->setValue(1);
    quantityLayout->addWidget(m_quantitySpinBox);
    quantityLayout->addStretch();
    purchaseLayout->addLayout(quantityLayout);

    m_buyBtn->setMinimumHeight(50);
    m_buyBtn->setStyleSheet("background-color: #e74c3c; color: white; font-size: 18px; font-weight: bold;");
    purchaseLayout->addWidget(m_buyBtn);

    mainLayout->addWidget(purchaseGroup);

    mainLayout->addStretch();

    setCentralWidget(centralWidget);
}

void ActivityDetailWindow::onCountdownReceived(int countdown, const QString& status)
{
    qDebug() << "onCountdownReceived: countdown=" << countdown << "status=" << status;
    m_countdown = countdown;
    m_status = status;
    updateCountdownDisplay();

    // If activity is ended, stop the timer
    if (status == "2") {
        m_countdownTimer->stop();
    }
}

void ActivityDetailWindow::onStockReceived(int remainStock)
{
    m_remainStock = remainStock;
    updateStockDisplay();
}

void ActivityDetailWindow::onCountdownTimerUpdate()
{
    if (m_countdown > 0) {
        m_countdown--;
        updateCountdownDisplay();
    } else if (m_status == "1") {
        // Activity is active, re-fetch countdown to check if it ends
        NetworkManager::instance().getCountdown(m_activity["id"].toInt());
    }
}

void ActivityDetailWindow::updateCountdownDisplay()
{
    if (m_status == "2") {
        // Activity ended
        m_countdownLabel->setText("活动已结束");
        m_countdownLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #95a5a6;");
        m_buyBtn->setEnabled(false);
        m_buyBtn->setText("已结束");
        m_countdownTimer->stop();
    } else if (m_countdown <= 0) {
        // Activity active (status=1) and countdown reached 0
        m_countdownLabel->setText("进行中");
        m_countdownLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #27ae60;");
        if (m_remainStock > 0) {
            m_buyBtn->setEnabled(true);
            m_buyBtn->setText("立即抢购");
        } else {
            m_buyBtn->setEnabled(false);
            m_buyBtn->setText("已售罄");
        }
    } else {
        // Countdown before start
        int hours = m_countdown / 3600;
        int minutes = (m_countdown % 3600) / 60;
        int seconds = m_countdown % 60;
        m_countdownLabel->setText(QString("%1:%2:%3")
                                   .arg(hours, 2, 10, QChar('0'))
                                   .arg(minutes, 2, 10, QChar('0'))
                                   .arg(seconds, 2, 10, QChar('0')));
        m_buyBtn->setEnabled(false);
        m_buyBtn->setText("等待开始");
    }
}

void ActivityDetailWindow::updateStockDisplay()
{
    m_stockLabel->setText(QString::number(m_remainStock));
    if (m_remainStock <= 0) {
        m_stockLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #95a5a6;");
        m_buyBtn->setEnabled(false);
        m_buyBtn->setText("已售罄");
    }
}

void ActivityDetailWindow::onBuyClicked()
{
    // Immediately disable button to prevent multiple clicks
    m_buyBtn->setEnabled(false);

    // Double-check status before purchase
    if (m_status == "2") {
        QMessageBox::warning(this, "提示", "活动已结束！");
        m_buyBtn->setText("已结束");
        return;
    }

    if (m_countdown > 0) {
        QMessageBox::warning(this, "提示", "活动尚未开始，请等待倒计时结束！");
        m_buyBtn->setText("等待开始");
        return;
    }

    if (m_remainStock <= 0) {
        QMessageBox::warning(this, "提示", "库存不足，已售罄！");
        m_buyBtn->setText("已售罄");
        return;
    }

    int activityId = m_activity["id"].toInt();
    int userId = NetworkManager::instance().getUserId();
    int quantity = m_quantitySpinBox->value();

    m_buyBtn->setText("抢购中...");

    NetworkManager::instance().createSeckillOrder(activityId, userId, quantity);
}

void ActivityDetailWindow::onOrderCreated(const QJsonObject& result)
{
    QString orderNo = result["order_no"].toString();
    QString message = result["message"].toString();

    QMessageBox::information(this, "抢购成功",
        QString("订单号: %1\n状态: %2").arg(orderNo).arg(message));
    m_buyBtn->setText("立即抢购");
    m_buyBtn->setEnabled(true);

    // Refresh stock
    NetworkManager::instance().getActivityStock(m_activity["id"].toInt());
}

void ActivityDetailWindow::onOrderStatusReceived(const QString& status, const QString& statusText)
{
    QMessageBox::information(this, "订单状态", QString("状态: %1").arg(statusText));
}

void ActivityDetailWindow::onErrorOccurred(const QString& error)
{
    qDebug() << "onErrorOccurred:" << error;
    QMessageBox::critical(this, "错误", error);
    m_buyBtn->setText("立即抢购");
    m_buyBtn->setEnabled(true);
}
