#include "StatusMonitor.h"
#include "Oxford133.h"

StatusMonitor::StatusMonitor(Oxford133 *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
    , m_timer(new QTimer(this))
    , m_lastRFPower(0)
    , m_lastICPPower(0)
    , m_lastPressure(0)
    , m_lastTemperature(0)
{
    memset(&m_status, 0, sizeof(m_status));

    connect(m_timer, &QTimer::timeout, this, &StatusMonitor::pollStatus);
}

StatusMonitor::~StatusMonitor()
{
    stop();
}

void StatusMonitor::start(int intervalMs)
{
    m_timer->start(intervalMs);
}

void StatusMonitor::stop()
{
    m_timer->stop();
}

void StatusMonitor::pollStatus()
{
    if (!m_device || !m_device->isConnected()) {
        return;
    }

    // 查询状态寄存器
    QString statusStr = m_device->query("STATus:OPERation?");
    if (!statusStr.isEmpty()) {
        // 解析状态位 (需要根据实际协议确定解析方式)
        // 这里假设返回的是十进制数字，每位代表一个状态
        bool ok;
        int statusValue = statusStr.toInt(&ok);
        if (ok) {
            // STS1
            m_status.running = (statusValue & 0x0001);
            m_status.inhibit = (statusValue & 0x0002);
            m_status.complete = (statusValue & 0x0004);
            m_status.aborted = (statusValue & 0x0008);
            m_status.error = (statusValue & 0x0010);

            // STS2
            m_status.processing = (statusValue & 0x0100);
            m_status.idle = (statusValue & 0x0200);
            m_status.waitingForGas = (statusValue & 0x0400);
            m_status.timeStarted = (statusValue & 0x0800);
            m_status.endpointDetected = (statusValue & 0x1000);

            emit statusUpdated(m_status);
        }
    }

    // 查询 RF 功率
    double rfPower = m_device->getRFPower();
    if (rfPower >= 0 && rfPower != m_lastRFPower) {
        m_lastRFPower = rfPower;
        emit rfPowerChanged(rfPower);
    }

    // 查询 ICP 功率
    double icpPower = m_device->getICPPower();
    if (icpPower >= 0 && icpPower != m_lastICPPower) {
        m_lastICPPower = icpPower;
        emit icpPowerChanged(icpPower);
    }

    // 查询压力
    double pressure = m_device->getPressure();
    if (pressure >= 0 && pressure != m_lastPressure) {
        m_lastPressure = pressure;
        emit pressureChanged(pressure);
    }

    // 查询温度
    double temperature = m_device->getTemperature();
    if (temperature >= 0 && temperature != m_lastTemperature) {
        m_lastTemperature = temperature;
        emit temperatureChanged(temperature);
    }

    // 查询气体流量 (最多 10 通道，这里查询前 4 个常用通道)
    for (int ch = 1; ch <= 4; ++ch) {
        double flow = m_device->getGasFlow(ch);
        if (flow >= 0) {
            emit gasFlowChanged(ch, flow);
        }
    }
}
