#include "Oxford133.h"
#include <QHostAddress>
#include <QTimer>
#include <QDebug>

Oxford133::Oxford133(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_connected(false)
{
    connect(m_socket, &QTcpSocket::connected, this, &Oxford133::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Oxford133::disconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [this](QAbstractSocket::SocketError){
        m_lastError = m_socket->errorString();
        emit errorOccurred(m_lastError);
    });
}

Oxford133::~Oxford133()
{
    disconnectFromDevice();
}

bool Oxford133::connectToDevice(const QString &host, quint16 port)
{
    QMutexLocker locker(&m_mutex);

    if (m_connected) {
        disconnectFromDevice();
    }

    m_socket->connectToHost(host, port);
    if (m_socket->waitForConnected(5000)) {
        m_connected = true;
        return true;
    }

    m_lastError = QStringLiteral("连接失败: %1").arg(m_socket->errorString());
    return false;
}

void Oxford133::disconnectFromDevice()
{
    QMutexLocker locker(&m_mutex);

    if (m_socket->isOpen()) {
        m_socket->disconnectFromHost();
    }
    m_connected = false;
}

bool Oxford133::isConnected() const
{
    return m_connected && m_socket->state() == QAbstractSocket::ConnectedState;
}

QString Oxford133::sendCommand(const QString &cmd, int timeoutMs)
{
    QMutexLocker locker(&m_mutex);

    if (!isConnected()) {
        m_lastError = QStringLiteral("设备未连接");
        return QString();
    }

    // SCPI 命令以换行符结束
    QByteArray data = (cmd + "\n").toUtf8();
    m_socket->write(data);
    m_socket->flush();

    emit commandSent(cmd);

    // 等待响应
    if (m_socket->waitForReadyRead(timeoutMs)) {
        QByteArray response = m_socket->readAll();
        QString respStr = QString::fromUtf8(response).trimmed();
        emit responseReceived(respStr);
        return respStr;
    }

    m_lastError = QStringLiteral("命令超时: %1").arg(cmd);
    return QString();
}

QString Oxford133::query(const QString &cmd, int timeoutMs)
{
    return sendCommand(cmd, timeoutMs);
}

bool Oxford133::setRFPower(double power)
{
    if (power < 0 || power > 600) {
        m_lastError = QStringLiteral("RF功率超出范围 0-600W");
        return false;
    }
    QString cmd = QString("SOURce:RF:POWer %1").arg(power);
    QString resp = sendCommand(cmd);
    return !resp.isEmpty();
}

double Oxford133::getRFPower()
{
    QString resp = query("SOURce:RF:POWer?");
    if (resp.isEmpty()) return -1;
    return resp.toDouble();
}

bool Oxford133::setICPPower(double power)
{
    if (power < 0 || power > 2000) {
        m_lastError = QStringLiteral("ICP功率超出范围 0-2000W");
        return false;
    }
    QString cmd = QString("SOURce:ICP:POWer %1").arg(power);
    QString resp = sendCommand(cmd);
    return !resp.isEmpty();
}

double Oxford133::getICPPower()
{
    QString resp = query("SOURce:ICP:POWer?");
    if (resp.isEmpty()) return -1;
    return resp.toDouble();
}

bool Oxford133::setPressure(double pressure)
{
    if (pressure < 0.1 || pressure > 100) {
        m_lastError = QStringLiteral("压力超出范围 0.1-100 mbar");
        return false;
    }
    QString cmd = QString("SOURce:PRESsure %1").arg(pressure);
    QString resp = sendCommand(cmd);
    return !resp.isEmpty();
}

double Oxford133::getPressure()
{
    QString resp = query("MEASure:PRESsure?");
    if (resp.isEmpty()) return -1;
    return resp.toDouble();
}

bool Oxford133::setGasFlow(int channel, double flow)
{
    if (channel < 1 || channel > 10) {
        m_lastError = QStringLiteral("气体通道超出范围 1-10");
        return false;
    }
    if (flow < 0 || flow > 200) {
        m_lastError = QStringLiteral("气体流量超出范围 0-200 seem");
        return false;
    }
    QString cmd = QString("SOURce:GAS%1:FLOw %2").arg(channel).arg(flow);
    QString resp = sendCommand(cmd);
    return !resp.isEmpty();
}

double Oxford133::getGasFlow(int channel)
{
    if (channel < 1 || channel > 10) return -1;
    QString resp = query(QString("MEASure:GAS%1:FLOw?").arg(channel));
    if (resp.isEmpty()) return -1;
    return resp.toDouble();
}

bool Oxford133::setTemperature(double temp)
{
    if (temp < -40 || temp > 180) {
        m_lastError = QStringLiteral("温度超出范围 -40-180°C");
        return false;
    }
    QString cmd = QString("SOURce:TEMPerature %1").arg(temp);
    QString resp = sendCommand(cmd);
    return !resp.isEmpty();
}

double Oxford133::getTemperature()
{
    QString resp = query("MEASure:TEMPerature?");
    if (resp.isEmpty()) return -1;
    return resp.toDouble();
}

bool Oxford133::startProcess()
{
    QString resp = sendCommand("INITiate");
    return !resp.isEmpty();
}

bool Oxford133::stopProcess()
{
    QString resp = sendCommand("ABORt");
    return !resp.isEmpty();
}

bool Oxford133::abortProcess()
{
    return stopProcess();
}

QString Oxford133::getSystemVersion()
{
    return query("SYSTem:VERSion?");
}

QString Oxford133::getStatus()
{
    return query("STATus:OPERation?");
}

QString Oxford133::getErrorMessage()
{
    return m_lastError;
}
