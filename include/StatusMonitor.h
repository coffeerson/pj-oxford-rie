#ifndef STATUSMONITOR_H
#define STATUSMONITOR_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>

/**
 * StatusMonitor - 设备状态监控线程
 * 定期轮询设备状态 (STS1-STS4)
 */
class Oxford133;

class StatusMonitor : public QObject
{
    Q_OBJECT

public:
    explicit StatusMonitor(Oxford133 *device, QObject *parent = nullptr);
    ~StatusMonitor();

    void start(int intervalMs = 1000);   // 开始监控
    void stop();                          // 停止监控

    // 状态位定义
    struct StatusBits {
        // STS1
        bool running;
        bool inhibit;
        bool complete;
        bool aborted;
        bool error;

        // STS2
        bool processing;
        bool idle;
        bool waitingForGas;
        bool timeStarted;
        bool endpointDetected;

        // STS3
        bool pressureOK;
        bool pressureWarning;
        bool pressureFault;

        // STS4
        bool waterFlowOK;
        bool temperatureOK;
        bool rfOK;
        bool vacuumOK;
    };

    StatusBits getCurrentStatus() const { return m_status; }

signals:
    void statusUpdated(const StatusBits &status);
    void rfPowerChanged(double power);
    void icpPowerChanged(double power);
    void pressureChanged(double pressure);
    void temperatureChanged(double temperature);
    void gasFlowChanged(int channel, double flow);

private slots:
    void pollStatus();

private:
    Oxford133 *m_device;
    QTimer *m_timer;
    StatusBits m_status;

    double m_lastRFPower;
    double m_lastICPPower;
    double m_lastPressure;
    double m_lastTemperature;
};

#endif // STATUSMONITOR_H
