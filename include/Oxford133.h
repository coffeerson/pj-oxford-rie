#ifndef OXFORD133_H
#define OXFORD133_H

#include <QObject>
#include <QTcpSocket>
#include <QMutex>
#include <QMutexLocker>

/**
 * Oxford133 - Oxford ICP133 RIE 设备 SCPI 通信类
 * 通过 TCP/IP (端口 5025) 发送 SCPI 命令控制设备
 */
class Oxford133 : public QObject
{
    Q_OBJECT

public:
    explicit Oxford133(QObject *parent = nullptr);
    ~Oxford133();

    // 连接管理
    bool connectToDevice(const QString &host, quint16 port = 5025);
    void disconnectFromDevice();
    bool isConnected() const;

    // SCPI 命令发送
    QString sendCommand(const QString &cmd, int timeoutMs = 3000);
    QString query(const QString &cmd, int timeoutMs = 3000);

    // 工艺参数设置
    bool setRFPower(double power);       // RF Bias 功率 0-600W
    double getRFPower();
    bool setICPPower(double power);      // ICP Source 功率 0-2000W
    double getICPPower();
    bool setPressure(double pressure);   // 压力 0.1-100 mbar
    double getPressure();
    bool setGasFlow(int channel, double flow);  // 气体流量
    double getGasFlow(int channel);
    bool setTemperature(double temp);     // Chuck 温度
    double getTemperature();

    // 工艺控制
    bool startProcess();
    bool stopProcess();
    bool abortProcess();

    // 状态查询
    QString getSystemVersion();
    QString getStatus();
    QString getErrorMessage();

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);
    void commandSent(const QString &cmd);
    void responseReceived(const QString &response);

private:
    QTcpSocket *m_socket;
    QMutex m_mutex;
    bool m_connected;
    QString m_lastError;
};

#endif // OXFORD133_H
