#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QGroupBox>
#include <QSplitter>
#include <QTimer>

#include "StatusMonitor.h"
#include "RecipeManager.h"
#include "HardwareDiagram.h"

class Oxford133;
class DataLogger;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 连接管理
    void onConnectClicked();
    void onDisconnectClicked();
    void onConnectionStatusChanged();

    // 手动控制
    void onSetRFPowerClicked();
    void onSetICPPowerClicked();
    void onSetPressureClicked();
    void onSetGasFlowClicked();
    void onSetTemperatureClicked();
    void onStartProcessClicked();
    void onStopProcessClicked();

    // 配方管理
    void onRecipeSelectionChanged(const QString &recipeName);
    void onRunRecipeClicked();
    void onStopRecipeClicked();
    void onSaveRecipeClicked();
    void onLoadRecipeClicked();
    void onNewRecipeClicked();

    // 状态更新
    void onStatusUpdated(const StatusMonitor::StatusBits &status);
    void onRFPowerChanged(double power);
    void onICPPowerChanged(double power);
    void onPressureChanged(double pressure);
    void onTemperatureChanged(double temperature);
    void onGasFlowChanged(int channel, double flow);

    // 日志
    void onCommandSent(const QString &cmd);
    void onResponseReceived(const QString &response);
    void onClearLogClicked();

    // 配方步骤更新
    void onRecipeStepStarted(int stepNumber, const RecipeManager::RecipeStep &step);
    void onRecipeCompleted(const QString &recipeName);
    void onRecipeProgress(int currentStep, int totalSteps, int currentCycle, int totalCycles);

private:
    void setupUi();
    void createConnectionGroup();
    QGroupBox* createStatusGroup();
    QGroupBox* createManualControlGroup();
    QGroupBox* createRecipeGroup();
    void createLogGroup();

    void updateConnectionStatus();
    void updateRecipeComboBox();
    void appendLog(const QString &text);

    Oxford133 *m_device;
    StatusMonitor *m_statusMonitor;
    RecipeManager *m_recipeManager;
    DataLogger *m_dataLogger;
    HardwareDiagram *m_hardwareDiagram;

    // 连接相关
    QLineEdit *m_hostEdit;
    QSpinBox *m_portSpinBox;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QLabel *m_connectionStatusLabel;

    // 状态显示
    QLabel *m_rfPowerLabel;
    QLabel *m_icpPowerLabel;
    QLabel *m_pressureLabel;
    QLabel *m_temperatureLabel;
    QLabel *m_gasFlowLabel[4];
    QLabel *m_statusLabel;

    // 手动控制
    QDoubleSpinBox *m_rfPowerSpinBox;
    QDoubleSpinBox *m_icpPowerSpinBox;
    QDoubleSpinBox *m_pressureSpinBox;
    QDoubleSpinBox *m_temperatureSpinBox;
    QDoubleSpinBox *m_gasFlowSpinBox[4];
    QComboBox *m_gasFlowChannelComboBox;

    // 配方管理
    QComboBox *m_recipeComboBox;
    QTableWidget *m_recipeTable;
    QPushButton *m_newRecipeBtn;
    QPushButton *m_saveRecipeBtn;
    QPushButton *m_loadRecipeBtn;
    QPushButton *m_runRecipeBtn;
    QPushButton *m_stopRecipeBtn;
    QLabel *m_recipeProgressLabel;

    // 日志
    QTextEdit *m_logTextEdit;
    QPushButton *m_clearLogBtn;

    // 定时器
    QTimer *m_uiUpdateTimer;
};

#endif // MAINWINDOW_H
