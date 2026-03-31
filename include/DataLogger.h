#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QVector>
#include <QDateTime>

/**
 * DataLogger - 工艺数据记录类
 * 记录工艺过程中的参数到 CSV 文件
 */
class Oxford133;
class StatusMonitor;

class DataLogger : public QObject
{
    Q_OBJECT

public:
    struct DataPoint {
        QDateTime timestamp;
        double rfPower;
        double icpPower;
        double pressure;
        double temperature;
        double gasFlow1;
        double gasFlow2;
        double gasFlow3;
        double gasFlow4;
    };

    explicit DataLogger(Oxford133 *device, QObject *parent = nullptr);
    ~DataLogger();

    // 文件操作
    bool startLogging(const QString &filePath);
    void stopLogging();
    bool isLogging() const { return m_logging; }

    // 数据点管理
    void addDataPoint(const DataPoint &point);
    QVector<DataPoint> getDataPoints() const { return m_dataPoints; }
    void clearDataPoints();

    // 导出
    bool exportToCsv(const QString &filePath) const;

signals:
    void loggingStarted();
    void loggingStopped();
    void dataPointAdded(const DataPoint &point);

private slots:
    void onTimeout();

private:
    Oxford133 *m_device;
    QTimer *m_timer;
    QFile *m_file;
    QTextStream *m_stream;
    bool m_logging;
    QVector<DataPoint> m_dataPoints;
};

#endif // DATALOGGER_H
