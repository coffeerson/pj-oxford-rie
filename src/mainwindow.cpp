#include "mainwindow.h"
#include "Oxford133.h"
#include "StatusMonitor.h"
#include "RecipeManager.h"
#include "DataLogger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QTabWidget>
#include <QWidget>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QAction>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_device(new Oxford133(this))
    , m_statusMonitor(new StatusMonitor(m_device, this))
    , m_recipeManager(new RecipeManager(m_device, this))
    , m_dataLogger(new DataLogger(m_device, this))
    , m_hardwareDiagram(nullptr)
{
    setupUi();

    // 连接信号
    connect(m_device, &Oxford133::connected, this, &MainWindow::onConnectionStatusChanged);
    connect(m_device, &Oxford133::disconnected, this, &MainWindow::onConnectionStatusChanged);
    connect(m_device, &Oxford133::commandSent, this, &MainWindow::onCommandSent);
    connect(m_device, &Oxford133::responseReceived, this, &MainWindow::onResponseReceived);

    connect(m_statusMonitor, &StatusMonitor::statusUpdated, this, &MainWindow::onStatusUpdated);
    connect(m_statusMonitor, &StatusMonitor::rfPowerChanged, this, &MainWindow::onRFPowerChanged);
    connect(m_statusMonitor, &StatusMonitor::icpPowerChanged, this, &MainWindow::onICPPowerChanged);
    connect(m_statusMonitor, &StatusMonitor::pressureChanged, this, &MainWindow::onPressureChanged);
    connect(m_statusMonitor, &StatusMonitor::temperatureChanged, this, &MainWindow::onTemperatureChanged);
    connect(m_statusMonitor, &StatusMonitor::gasFlowChanged, this, &MainWindow::onGasFlowChanged);

    connect(m_recipeManager, &RecipeManager::recipeStepStarted, this, &MainWindow::onRecipeStepStarted);
    connect(m_recipeManager, &RecipeManager::recipeCompleted, this, &MainWindow::onRecipeCompleted);
    connect(m_recipeManager, &RecipeManager::progressUpdated, this, &MainWindow::onRecipeProgress);

    // 创建默认配方
    m_recipeManager->createDefaultRecipes();
    updateRecipeComboBox();

    // 启动状态监控
    m_statusMonitor->start(1000);

    // UI 定时更新
    m_uiUpdateTimer = new QTimer(this);
    connect(m_uiUpdateTimer, &QTimer::timeout, this, [this]() {
        updateConnectionStatus();
    });
    m_uiUpdateTimer->start(500);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    // 设置窗口标题和大小（模仿 Windows Classic 风格）
    setWindowTitle("Oxford Plasmalab System 133 - PC 2000");
    setGeometry(100, 100, 1100, 780);

    // 创建硬件框架图（这是主界面）
    m_hardwareDiagram = new HardwareDiagram(this);

    // 直接设置为中央控件，不再使用 Tab
    setCentralWidget(m_hardwareDiagram);

    // 连接状态监控信号到硬件框架图更新
    connect(m_statusMonitor, &StatusMonitor::rfPowerChanged, this, &MainWindow::onRFPowerChanged);
    connect(m_statusMonitor, &StatusMonitor::icpPowerChanged, this, &MainWindow::onICPPowerChanged);
    connect(m_statusMonitor, &StatusMonitor::pressureChanged, this, &MainWindow::onPressureChanged);
    connect(m_statusMonitor, &StatusMonitor::temperatureChanged, this, &MainWindow::onTemperatureChanged);

    // 添加工具栏（快速访问常用功能）
    QToolBar *toolbar = addToolBar(QStringLiteral("工具栏"));
    toolbar->setMovable(false);

    QAction *connectAction = new QAction(QStringLiteral("连接"), this);
    QAction *disconnectAction = new QAction(QStringLiteral("断开"), this);
    QAction *startAction = new QAction(QStringLiteral("开始工艺"), this);
    QAction *stopAction = new QAction(QStringLiteral("停止"), this);

    toolbar->addAction(connectAction);
    toolbar->addAction(disconnectAction);
    toolbar->addSeparator();
    toolbar->addAction(startAction);
    toolbar->addAction(stopAction);

    connect(connectAction, &QAction::triggered, this, &MainWindow::onConnectClicked);
    connect(disconnectAction, &QAction::triggered, this, &MainWindow::onDisconnectClicked);
    connect(startAction, &QAction::triggered, this, &MainWindow::onStartProcessClicked);
    connect(stopAction, &QAction::triggered, this, &MainWindow::onStopProcessClicked);

    // 底部状态栏
    statusBar()->showMessage(QStringLiteral("未连接"));
}

void MainWindow::createConnectionGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("连接管理"), this);
    QHBoxLayout *layout = new QHBoxLayout(group);

    m_hostEdit = new QLineEdit("192.168.1.100", this);
    m_hostEdit->setPlaceholderText(QStringLiteral("IP地址"));
    layout->addWidget(new QLabel(QStringLiteral("主机:")));
    layout->addWidget(m_hostEdit);

    m_portSpinBox = new QSpinBox(this);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(5025);
    layout->addWidget(new QLabel(QStringLiteral("端口:")));
    layout->addWidget(m_portSpinBox);

    m_connectBtn = new QPushButton(QStringLiteral("连接"), this);
    m_disconnectBtn = new QPushButton(QStringLiteral("断开"), this);
    m_disconnectBtn->setEnabled(false);
    layout->addWidget(m_connectBtn);
    layout->addWidget(m_disconnectBtn);

    m_connectionStatusLabel = new QLabel(QStringLiteral("未连接"), this);
    m_connectionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    layout->addWidget(m_connectionStatusLabel);

    layout->addStretch();

    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
}

QGroupBox *MainWindow::createStatusGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("设备状态"), this);
    QGridLayout *layout = new QGridLayout(group);

    m_rfPowerLabel = new QLabel("0.0 W", this);
    m_icpPowerLabel = new QLabel("0.0 W", this);
    m_pressureLabel = new QLabel("0.00 mbar", this);
    m_temperatureLabel = new QLabel("0.0 °C", this);
    m_gasFlowLabel[0] = new QLabel("0.0 seem", this);
    m_gasFlowLabel[1] = new QLabel("0.0 seem", this);
    m_gasFlowLabel[2] = new QLabel("0.0 seem", this);
    m_gasFlowLabel[3] = new QLabel("0.0 seem", this);
    m_statusLabel = new QLabel(QStringLiteral("未知"), this);

    layout->addWidget(new QLabel(QStringLiteral("RF 功率:")), 0, 0);
    layout->addWidget(m_rfPowerLabel, 0, 1);
    layout->addWidget(new QLabel(QStringLiteral("ICP 功率:")), 0, 2);
    layout->addWidget(m_icpPowerLabel, 0, 3);

    layout->addWidget(new QLabel(QStringLiteral("压力:")), 1, 0);
    layout->addWidget(m_pressureLabel, 1, 1);
    layout->addWidget(new QLabel(QStringLiteral("温度:")), 1, 2);
    layout->addWidget(m_temperatureLabel, 1, 3);

    layout->addWidget(new QLabel(QStringLiteral("气体1:")), 2, 0);
    layout->addWidget(m_gasFlowLabel[0], 2, 1);
    layout->addWidget(new QLabel(QStringLiteral("气体2:")), 2, 2);
    layout->addWidget(m_gasFlowLabel[1], 2, 3);

    layout->addWidget(new QLabel(QStringLiteral("气体3:")), 3, 0);
    layout->addWidget(m_gasFlowLabel[2], 3, 1);
    layout->addWidget(new QLabel(QStringLiteral("气体4:")), 3, 2);
    layout->addWidget(m_gasFlowLabel[3], 3, 3);

    layout->addWidget(new QLabel(QStringLiteral("状态:")), 4, 0);
    layout->addWidget(m_statusLabel, 4, 1, 1, 3);

    return group;
}

QGroupBox *MainWindow::createManualControlGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("手动控制"), this);
    QGridLayout *layout = new QGridLayout(group);

    // RF 功率
    m_rfPowerSpinBox = new QDoubleSpinBox(this);
    m_rfPowerSpinBox->setRange(0, 600);
    m_rfPowerSpinBox->setSingleStep(1);
    m_rfPowerSpinBox->setSuffix(" W");
    QPushButton *rfSetBtn = new QPushButton(QStringLiteral("设置"), this);
    layout->addWidget(new QLabel(QStringLiteral("RF功率:")), 0, 0);
    layout->addWidget(m_rfPowerSpinBox, 0, 1);
    layout->addWidget(rfSetBtn, 0, 2);
    connect(rfSetBtn, &QPushButton::clicked, this, &MainWindow::onSetRFPowerClicked);

    // ICP 功率
    m_icpPowerSpinBox = new QDoubleSpinBox(this);
    m_icpPowerSpinBox->setRange(0, 2000);
    m_icpPowerSpinBox->setSingleStep(10);
    m_icpPowerSpinBox->setSuffix(" W");
    QPushButton *icpSetBtn = new QPushButton(QStringLiteral("设置"), this);
    layout->addWidget(new QLabel(QStringLiteral("ICP功率:")), 1, 0);
    layout->addWidget(m_icpPowerSpinBox, 1, 1);
    layout->addWidget(icpSetBtn, 1, 2);
    connect(icpSetBtn, &QPushButton::clicked, this, &MainWindow::onSetICPPowerClicked);

    // 压力
    m_pressureSpinBox = new QDoubleSpinBox(this);
    m_pressureSpinBox->setRange(0.1, 100);
    m_pressureSpinBox->setSingleStep(0.1);
    m_pressureSpinBox->setSuffix(" mbar");
    QPushButton *pressureSetBtn = new QPushButton(QStringLiteral("设置"), this);
    layout->addWidget(new QLabel(QStringLiteral("压力:")), 2, 0);
    layout->addWidget(m_pressureSpinBox, 2, 1);
    layout->addWidget(pressureSetBtn, 2, 2);
    connect(pressureSetBtn, &QPushButton::clicked, this, &MainWindow::onSetPressureClicked);

    // 温度
    m_temperatureSpinBox = new QDoubleSpinBox(this);
    m_temperatureSpinBox->setRange(-40, 180);
    m_temperatureSpinBox->setSingleStep(1);
    m_temperatureSpinBox->setSuffix(" °C");
    QPushButton *tempSetBtn = new QPushButton(QStringLiteral("设置"), this);
    layout->addWidget(new QLabel(QStringLiteral("温度:")), 3, 0);
    layout->addWidget(m_temperatureSpinBox, 3, 1);
    layout->addWidget(tempSetBtn, 3, 2);
    connect(tempSetBtn, &QPushButton::clicked, this, &MainWindow::onSetTemperatureClicked);

    // 气体流量
    m_gasFlowChannelComboBox = new QComboBox(this);
    for (int i = 1; i <= 4; ++i) {
        m_gasFlowChannelComboBox->addItem(QString("CH%1").arg(i), i);
    }
    m_gasFlowSpinBox[0] = new QDoubleSpinBox(this);
    m_gasFlowSpinBox[0]->setRange(0, 200);
    m_gasFlowSpinBox[0]->setSingleStep(1);
    m_gasFlowSpinBox[0]->setSuffix(" seem");
    QPushButton *gasSetBtn = new QPushButton(QStringLiteral("设置"), this);
    layout->addWidget(new QLabel(QStringLiteral("气体:")), 4, 0);
    layout->addWidget(m_gasFlowChannelComboBox, 4, 1);
    layout->addWidget(m_gasFlowSpinBox[0], 4, 2);
    layout->addWidget(gasSetBtn, 4, 3);
    connect(gasSetBtn, &QPushButton::clicked, this, &MainWindow::onSetGasFlowClicked);

    // 工艺控制按钮
    QPushButton *startBtn = new QPushButton(QStringLiteral("启动工艺"), this);
    QPushButton *stopBtn = new QPushButton(QStringLiteral("停止工艺"), this);
    layout->addWidget(startBtn, 5, 0, 1, 2);
    layout->addWidget(stopBtn, 5, 2, 1, 2);
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartProcessClicked);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopProcessClicked);

    return group;
}

QGroupBox *MainWindow::createRecipeGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("配方管理"), this);
    QVBoxLayout *layout = new QVBoxLayout(group);

    // 配方选择
    QHBoxLayout *selectLayout = new QHBoxLayout();
    selectLayout->addWidget(new QLabel(QStringLiteral("配方:")));
    m_recipeComboBox = new QComboBox(this);
    selectLayout->addWidget(m_recipeComboBox);
    layout->addLayout(selectLayout);

    // 配方表格
    m_recipeTable = new QTableWidget(10, 9, this);
    m_recipeTable->setHorizontalHeaderLabels(QStringList() << "步骤" << "RF(W)"
                                              << "ICP(W)" << "压力" << "气体1"
                                              << "气体2" << "气体3" << "气体4"
                                              << "时间(s)");
    layout->addWidget(m_recipeTable);

    // 配方进度
    m_recipeProgressLabel = new QLabel(this);
    layout->addWidget(m_recipeProgressLabel);

    // 配方操作按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_newRecipeBtn = new QPushButton(QStringLiteral("新建"), this);
    m_saveRecipeBtn = new QPushButton(QStringLiteral("保存"), this);
    m_loadRecipeBtn = new QPushButton(QStringLiteral("加载"), this);
    m_runRecipeBtn = new QPushButton(QStringLiteral("运行配方"), this);
    m_stopRecipeBtn = new QPushButton(QStringLiteral("停止配方"), this);
    m_stopRecipeBtn->setEnabled(false);

    btnLayout->addWidget(m_newRecipeBtn);
    btnLayout->addWidget(m_saveRecipeBtn);
    btnLayout->addWidget(m_loadRecipeBtn);
    btnLayout->addWidget(m_runRecipeBtn);
    btnLayout->addWidget(m_stopRecipeBtn);
    layout->addLayout(btnLayout);

    connect(m_recipeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (index >= 0) {
            onRecipeSelectionChanged(m_recipeComboBox->itemText(index));
        }
    });
    connect(m_runRecipeBtn, &QPushButton::clicked, this, &MainWindow::onRunRecipeClicked);
    connect(m_stopRecipeBtn, &QPushButton::clicked, this, &MainWindow::onStopRecipeClicked);
    connect(m_saveRecipeBtn, &QPushButton::clicked, this, &MainWindow::onSaveRecipeClicked);
    connect(m_loadRecipeBtn, &QPushButton::clicked, this, &MainWindow::onLoadRecipeClicked);
    connect(m_newRecipeBtn, &QPushButton::clicked, this, &MainWindow::onNewRecipeClicked);

    return group;
}

void MainWindow::createLogGroup()
{
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Courier New", 9));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(new QLabel(QStringLiteral("通信日志:")));
    layout->addStretch();
    m_clearLogBtn = new QPushButton(QStringLiteral("清空日志"), this);
    layout->addWidget(m_clearLogBtn);
    connect(m_clearLogBtn, &QPushButton::clicked, this, &MainWindow::onClearLogClicked);
}

void MainWindow::updateRecipeComboBox()
{
    QStringList names = m_recipeManager->getRecipeNames();
    m_recipeComboBox->clear();
    m_recipeComboBox->addItems(names);
}

void MainWindow::onConnectClicked()
{
    QString host = m_hostEdit->text().trimmed();
    int port = m_portSpinBox->value();

    if (host.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("请输入主机地址"));
        return;
    }

    appendLog(QStringLiteral("[连接] 正在连接到 %1:%2...").arg(host).arg(port));

    if (m_device->connectToDevice(host, port)) {
        appendLog(QStringLiteral("[连接] 连接成功"));
        m_statusMonitor->start(1000);
    } else {
        appendLog(QStringLiteral("[错误] 连接失败: %1").arg(m_device->getErrorMessage()));
        QMessageBox::critical(this, QStringLiteral("连接错误"), m_device->getErrorMessage());
    }
}

void MainWindow::onDisconnectClicked()
{
    m_statusMonitor->stop();
    m_device->disconnectFromDevice();
    appendLog(QStringLiteral("[连接] 已断开连接"));
}

void MainWindow::onConnectionStatusChanged()
{
    bool connected = m_device->isConnected();
    m_connectBtn->setEnabled(!connected);
    m_disconnectBtn->setEnabled(connected);
    updateConnectionStatus();
}

void MainWindow::updateConnectionStatus()
{
    if (m_device->isConnected()) {
        m_connectionStatusLabel->setText(QStringLiteral("已连接"));
        m_connectionStatusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else {
        m_connectionStatusLabel->setText(QStringLiteral("未连接"));
        m_connectionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    }
}

void MainWindow::onSetRFPowerClicked()
{
    double power = m_rfPowerSpinBox->value();
    if (m_device->setRFPower(power)) {
        appendLog(QStringLiteral("[设置] RF功率: %1 W").arg(power));
    }
}

void MainWindow::onSetICPPowerClicked()
{
    double power = m_icpPowerSpinBox->value();
    if (m_device->setICPPower(power)) {
        appendLog(QStringLiteral("[设置] ICP功率: %1 W").arg(power));
    }
}

void MainWindow::onSetPressureClicked()
{
    double pressure = m_pressureSpinBox->value();
    if (m_device->setPressure(pressure)) {
        appendLog(QStringLiteral("[设置] 压力: %1 mbar").arg(pressure));
    }
}

void MainWindow::onSetTemperatureClicked()
{
    double temp = m_temperatureSpinBox->value();
    if (m_device->setTemperature(temp)) {
        appendLog(QStringLiteral("[设置] 温度: %1 °C").arg(temp));
    }
}

void MainWindow::onSetGasFlowClicked()
{
    int channel = m_gasFlowChannelComboBox->currentData().toInt();
    double flow = m_gasFlowSpinBox[0]->value();
    if (m_device->setGasFlow(channel, flow)) {
        appendLog(QStringLiteral("[设置] 气体%1: %2 seem").arg(channel).arg(flow));
    }
}

void MainWindow::onStartProcessClicked()
{
    if (m_device->startProcess()) {
        appendLog(QStringLiteral("[工艺] 启动工艺"));
    }
}

void MainWindow::onStopProcessClicked()
{
    if (m_device->stopProcess()) {
        appendLog(QStringLiteral("[工艺] 停止工艺"));
    }
}

void MainWindow::onRecipeSelectionChanged(const QString &recipeName)
{
    RecipeManager::Recipe recipe = m_recipeManager->getRecipe(recipeName);
    m_recipeTable->setRowCount(recipe.steps.size());

    for (int i = 0; i < recipe.steps.size(); ++i) {
        const RecipeManager::RecipeStep &step = recipe.steps[i];
        m_recipeTable->setItem(i, 0, new QTableWidgetItem(QString::number(step.stepNumber)));
        m_recipeTable->setItem(i, 1, new QTableWidgetItem(QString::number(step.rfPower)));
        m_recipeTable->setItem(i, 2, new QTableWidgetItem(QString::number(step.icpPower)));
        m_recipeTable->setItem(i, 3, new QTableWidgetItem(QString::number(step.pressure)));
        m_recipeTable->setItem(i, 4, new QTableWidgetItem(QString::number(step.gasFlow1)));
        m_recipeTable->setItem(i, 5, new QTableWidgetItem(QString::number(step.gasFlow2)));
        m_recipeTable->setItem(i, 6, new QTableWidgetItem(QString::number(step.gasFlow3)));
        m_recipeTable->setItem(i, 7, new QTableWidgetItem(QString::number(step.gasFlow4)));
        m_recipeTable->setItem(i, 8, new QTableWidgetItem(QString::number(step.time)));
    }
}

void MainWindow::onRunRecipeClicked()
{
    QString recipeName = m_recipeComboBox->currentText();
    if (recipeName.isEmpty()) return;

    m_recipeManager->setCurrentRecipe(recipeName);
    m_runRecipeBtn->setEnabled(false);
    m_stopRecipeBtn->setEnabled(true);
    m_recipeManager->runFullRecipe();
    appendLog(QStringLiteral("[配方] 开始运行: %1").arg(recipeName));
}

void MainWindow::onStopRecipeClicked()
{
    m_recipeManager->stopRecipe();
    m_runRecipeBtn->setEnabled(true);
    m_stopRecipeBtn->setEnabled(false);
    appendLog(QStringLiteral("[配方] 配方已停止"));
}

void MainWindow::onSaveRecipeClicked()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        QStringLiteral("保存配方"), "", "JSON Files (*.json)");
    if (!filePath.isEmpty()) {
        QString recipeName = m_recipeComboBox->currentText();
        if (m_recipeManager->saveToFile(filePath, recipeName)) {
            appendLog(QStringLiteral("[配方] 保存成功: %1").arg(filePath));
        }
    }
}

void MainWindow::onLoadRecipeClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        QStringLiteral("加载配方"), "", "JSON Files (*.json)");
    if (!filePath.isEmpty()) {
        if (m_recipeManager->loadFromFile(filePath)) {
            updateRecipeComboBox();
            appendLog(QStringLiteral("[配方] 加载成功: %1").arg(filePath));
        }
    }
}

void MainWindow::onNewRecipeClicked()
{
    RecipeManager::Recipe newRecipe;
    newRecipe.name = QStringLiteral("新配方_%1").arg(QDateTime::currentDateTime().toString("HHmmss"));
    newRecipe.cycles = 1;

    RecipeManager::RecipeStep step;
    step.stepNumber = 1;
    step.rfPower = 100;
    step.icpPower = 1000;
    step.pressure = 10;
    step.gasFlow1 = 50;
    step.gasFlow2 = 0;
    step.gasFlow3 = 0;
    step.gasFlow4 = 0;
    step.temperature = 20;
    step.time = 60;
    newRecipe.steps.append(step);

    m_recipeManager->addRecipe(newRecipe);
    updateRecipeComboBox();
    m_recipeComboBox->setCurrentText(newRecipe.name);
}

void MainWindow::onStatusUpdated(const StatusMonitor::StatusBits &status)
{
    QStringList statusList;
    if (status.running) statusList.append(QStringLiteral("运行中"));
    if (status.idle) statusList.append(QStringLiteral("空闲"));
    if (status.complete) statusList.append(QStringLiteral("完成"));
    if (status.error) statusList.append(QStringLiteral("错误"));
    if (status.aborted) statusList.append(QStringLiteral("已中止"));

    m_statusLabel->setText(statusList.join(" | "));

    // 更新硬件框架图状态
    if (m_hardwareDiagram) {
        if (status.running) {
            m_hardwareDiagram->updateComponentState("Chamber", ComponentState::Running);
            m_hardwareDiagram->updateComponentState("RF", ComponentState::Running);
            m_hardwareDiagram->updateComponentState("ICP", ComponentState::Running);
        } else {
            m_hardwareDiagram->updateComponentState("Chamber", ComponentState::Normal);
            m_hardwareDiagram->updateComponentState("RF", ComponentState::Normal);
            m_hardwareDiagram->updateComponentState("ICP", ComponentState::Normal);
        }
    }
}

void MainWindow::onRFPowerChanged(double power)
{
    m_rfPowerLabel->setText(QString::number(power, 'f', 1) + " W");
    if (m_hardwareDiagram) {
        m_hardwareDiagram->updateComponentValue("RF", QString::number(power, 'f', 0) + " W");
    }
}

void MainWindow::onICPPowerChanged(double power)
{
    m_icpPowerLabel->setText(QString::number(power, 'f', 1) + " W");
    if (m_hardwareDiagram) {
        m_hardwareDiagram->updateComponentValue("ICP", QString::number(power, 'f', 0) + " W");
    }
}

void MainWindow::onPressureChanged(double pressure)
{
    m_pressureLabel->setText(QString::number(pressure, 'f', 2) + " mbar");
    if (m_hardwareDiagram) {
        m_hardwareDiagram->updateComponentValue("Pressure", QString::number(pressure, 'f', 2) + " mbar");
    }
}

void MainWindow::onTemperatureChanged(double temperature)
{
    m_temperatureLabel->setText(QString::number(temperature, 'f', 1) + " °C");
    if (m_hardwareDiagram) {
        m_hardwareDiagram->updateComponentValue("Chuck", QString::number(temperature, 'f', 1) + " °C");
    }
}

void MainWindow::onGasFlowChanged(int channel, double flow)
{
    if (channel >= 1 && channel <= 4) {
        m_gasFlowLabel[channel - 1]->setText(QString::number(flow, 'f', 1) + " seem");
        if (m_hardwareDiagram) {
            QString mfcName = QString("MFC%1").arg(channel);
            m_hardwareDiagram->updateComponentValue(mfcName, QString::number(flow, 'f', 1) + " seem");
        }
    }
}

void MainWindow::onCommandSent(const QString &cmd)
{
    appendLog(QStringLiteral("[TX] %1").arg(cmd));
}

void MainWindow::onResponseReceived(const QString &response)
{
    appendLog(QStringLiteral("[RX] %1").arg(response));
}

void MainWindow::onClearLogClicked()
{
    m_logTextEdit->clear();
}

void MainWindow::appendLog(const QString &text)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    m_logTextEdit->append(QString("[%1] %2").arg(timestamp).arg(text));
}

void MainWindow::onRecipeStepStarted(int stepNumber, const RecipeManager::RecipeStep &step)
{
    appendLog(QStringLiteral("[配方] 步骤 %1: RF=%2W ICP=%3W P=%4mbar T=%5s")
        .arg(stepNumber).arg(step.rfPower).arg(step.icpPower)
        .arg(step.pressure).arg(step.time));
}

void MainWindow::onRecipeCompleted(const QString &recipeName)
{
    appendLog(QStringLiteral("[配方] 配方完成: %1").arg(recipeName));
    m_runRecipeBtn->setEnabled(true);
    m_stopRecipeBtn->setEnabled(false);
}

void MainWindow::onRecipeProgress(int currentStep, int totalSteps, int currentCycle, int totalCycles)
{
    m_recipeProgressLabel->setText(
        QStringLiteral("进度: 步骤 %1/%2, 循环 %3/%4")
            .arg(currentStep).arg(totalSteps).arg(currentCycle).arg(totalCycles));
}
