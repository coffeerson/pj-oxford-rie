#include "DataLogger.h"
#include "Oxford133.h"
#include <QDebug>

DataLogger::DataLogger(Oxford133 *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
    , m_timer(new QTimer(this))
    , m_file(nullptr)
    , m_stream(nullptr)
    , m_logging(false)
{
    connect(m_timer, &QTimer::timeout, this, &DataLogger::onTimeout);
}

DataLogger::~DataLogger()
{
    stopLogging();
}

bool DataLogger::startLogging(const QString &filePath)
{
    if (m_logging) {
        stopLogging();
    }

    m_file = new QFile(filePath, this);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        delete m_file;
        m_file = nullptr;
        return false;
    }

    m_stream = new QTextStream(m_file);

    // 写入 CSV 头
    *m_stream << "时间,RF功率(W),ICP功率(W),压力(mbar),温度(°C),"
              << "气体1(seem),气体2(seem),气体3(seem),气体4(seem)\n";

    m_timer->start(1000);  // 每秒记录一次
    m_logging = true;
    m_dataPoints.clear();

    emit loggingStarted();
    return true;
}

void DataLogger::stopLogging()
{
    if (!m_logging) return;

    m_timer->stop();

    if (m_stream) {
        m_stream->flush();
        delete m_stream;
        m_stream = nullptr;
    }

    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }

    m_logging = false;
    emit loggingStopped();
}

void DataLogger::addDataPoint(const DataPoint &point)
{
    m_dataPoints.append(point);

    if (m_logging && m_stream) {
        *m_stream << point.timestamp.toString("yyyy-MM-dd HH:mm:ss") << ","
                  << point.rfPower << ","
                  << point.icpPower << ","
                  << point.pressure << ","
                  << point.temperature << ","
                  << point.gasFlow1 << ","
                  << point.gasFlow2 << ","
                  << point.gasFlow3 << ","
                  << point.gasFlow4 << "\n";
    }

    emit dataPointAdded(point);
}

void DataLogger::clearDataPoints()
{
    m_dataPoints.clear();
}

bool DataLogger::exportToCsv(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);

    // 写入 CSV 头
    stream << "时间,RF功率(W),ICP功率(W),压力(mbar),温度(°C),"
           << "气体1(seem),气体2(seem),气体3(seem),气体4(seem)\n";

    // 写入数据
    for (const DataPoint &point : m_dataPoints) {
        stream << point.timestamp.toString("yyyy-MM-dd HH:mm:ss") << ","
               << point.rfPower << ","
               << point.icpPower << ","
               << point.pressure << ","
               << point.temperature << ","
               << point.gasFlow1 << ","
               << point.gasFlow2 << ","
               << point.gasFlow3 << ","
               << point.gasFlow4 << "\n";
    }

    file.close();
    return true;
}

void DataLogger::onTimeout()
{
    if (!m_device || !m_device->isConnected()) {
        return;
    }

    DataPoint point;
    point.timestamp = QDateTime::currentDateTime();
    point.rfPower = m_device->getRFPower();
    point.icpPower = m_device->getICPPower();
    point.pressure = m_device->getPressure();
    point.temperature = m_device->getTemperature();
    point.gasFlow1 = m_device->getGasFlow(1);
    point.gasFlow2 = m_device->getGasFlow(2);
    point.gasFlow3 = m_device->getGasFlow(3);
    point.gasFlow4 = m_device->getGasFlow(4);

    addDataPoint(point);
}
