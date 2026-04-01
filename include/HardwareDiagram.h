#ifndef HARDWarediagram_H
#define HARDWarediagram_H

#include <QWidget>
#include <QMap>
#include <QString>

/**
 * HardwareDiagram - Oxford ICP133 RIE 设备硬件框架图
 * 使用现代 Qt Widgets 架构
 */

class StatusPanel;
class MimicWidget;

// 组件状态
enum class ComponentState {
    Normal,   // 正常/停止
    Running,  // 运行中
    Warning,  // 警告
    Fault,    // 故障
    Unknown   // 未知
};

// 主框架图类 - 使用Qt Widgets
class HardwareDiagram : public QWidget
{
    Q_OBJECT

public:
    explicit HardwareDiagram(QWidget *parent = nullptr);
    ~HardwareDiagram();

    // 更新组件状态
    void updateComponentState(const QString &name, ComponentState state);

    // 更新组件值（压力、功率、温度等）
    void updateComponentValue(const QString &name, const QString &value);

    // 更新阀门状态
    void setValveOpen(const QString &name, bool open);

    // 更新管道流动状态
    void setPipeFlow(const QString &name, bool flowing);

    // 获取组件状态
    ComponentState getComponentState(const QString &name) const;

signals:
    void evacuateRequested();
    void stopRequested();
    void ventRequested();

public slots:
    void onEvacuateClicked();
    void onStopClicked();
    void onVentClicked();

private:
    void setupUi();

    // 状态面板
    StatusPanel *m_loadLockPanel;
    StatusPanel *m_processChamberPanel;
    StatusPanel *m_systemStatusPanel;

    // Mimic示意图
    MimicWidget *m_mimicWidget;

    // 组件状态映射
    QMap<QString, ComponentState> m_componentStates;
};

#endif // HARDWarediagram_H
