#ifndef HARDWarediagram_H
#define HARDWarediagram_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QMap>
#include <QString>

/**
 * HardwareDiagram - Oxford ICP133 RIE 设备硬件框架图
 * 使用 Qt Graphics View Framework 绘制设备示意图
 */

// 组件类型枚举
enum class ComponentType {
    Chamber,         // 真空腔室
    RFGenerator,     // RF 发生器
    ICPGenerator,    // ICP 发生器
    GasMFC,          // 气体质量流量控制器
    Valve,           // 阀门
    Pump,            // 泵
    PressureGauge,   // 压力计
    TemperatureSensor, // 温度传感器
    Chuck,           // 样品架/加热Chuck
    Pipe             // 管道
};

// 组件状态
enum class ComponentState {
    Normal,   // 正常/停止
    Running,  // 运行中
    Warning,  // 警告
    Fault,    // 故障
    Unknown   // 未知
};

// 硬件组件基类
class HardwareComponent : public QGraphicsItemGroup
{
public:
    HardwareComponent(const QString &name, ComponentType type, QGraphicsItem *parent = nullptr);

    QString getName() const { return m_name; }
    ComponentType getType() const { return m_type; }
    ComponentState getState() const { return m_state; }
    void setState(ComponentState state);

    // 设置显示的值
    void setValue(const QString &value);
    QString getValue() const { return m_value; }

    // 组件边界
    QRectF boundingRect() const override;

protected:
    QString m_name;
    ComponentType m_type;
    ComponentState m_state;
    QString m_value;
    QGraphicsTextItem *m_nameText;
    QGraphicsTextItem *m_valueText;

    virtual void updateAppearance() = 0;
};

// 腔室组件
class ChamberItem : public HardwareComponent
{
public:
    ChamberItem(const QString &name, QGraphicsItem *parent = nullptr);

protected:
    void updateAppearance() override;
};

// RF 发生器
class RFGeneratorItem : public HardwareComponent
{
public:
    RFGeneratorItem(const QString &name, QGraphicsItem *parent = nullptr);

protected:
    void updateAppearance() override;
};

// ICP 发生器
class ICPGeneratorItem : public HardwareComponent
{
public:
    ICPGeneratorItem(const QString &name, QGraphicsItem *parent = nullptr);

protected:
    void updateAppearance() override;
};

// 气体 MFC
class GasMFCItem : public HardwareComponent
{
public:
    GasMFCItem(const QString &name, int channel, QGraphicsItem *parent = nullptr);

    int getChannel() const { return m_channel; }

protected:
    void updateAppearance() override;
    int m_channel;
};

// 阀门
class ValveItem : public HardwareComponent
{
public:
    ValveItem(const QString &name, QGraphicsItem *parent = nullptr);

    void setOpen(bool open);
    bool isOpen() const { return m_open; }

protected:
    void updateAppearance() override;
    bool m_open;
};

// 泵
class PumpItem : public HardwareComponent
{
public:
    PumpItem(const QString &name, QGraphicsItem *parent = nullptr);

protected:
    void updateAppearance() override;
};

// 压力计
class PressureGaugeItem : public HardwareComponent
{
public:
    PressureGaugeItem(const QString &name, QGraphicsItem *parent = nullptr);

protected:
    void updateAppearance() override;
};

// 温度传感器/Chuck
class ChuckItem : public HardwareComponent
{
public:
    ChuckItem(const QString &name, QGraphicsItem *parent = nullptr);

protected:
    void updateAppearance() override;
};

// 管道
class PipeItem : public HardwareComponent
{
public:
    PipeItem(const QString &name, QGraphicsItem *parent = nullptr);

    void setFlowDirection(bool flowing);
    bool hasFlow() const { return m_flowing; }

protected:
    void updateAppearance() override;
    bool m_flowing;
};

// 主框架图类
class HardwareDiagram : public QGraphicsView
{
    Q_OBJECT

public:
    explicit HardwareDiagram(QWidget *parent = nullptr);
    ~HardwareDiagram();

    // 初始化设备组件布局
    void setupLayout();

    // 更新组件状态
    void updateComponentState(const QString &name, ComponentState state);

    // 更新组件值（压力、功率、温度等）
    void updateComponentValue(const QString &name, const QString &value);

    // 更新阀门状态
    void setValveOpen(const QString &name, bool open);

    // 更新管道流动状态
    void setPipeFlow(const QString &name, bool flowing);

    // 获取组件
    HardwareComponent* getComponent(const QString &name) const;

signals:
    void componentClicked(const QString &name, ComponentType type);

private slots:
    void onItemClicked(QGraphicsSceneMouseEvent *event);

private:
    QGraphicsScene *m_scene;

    // 组件映射
    QMap<QString, HardwareComponent*> m_components;
};

#endif // HARDWarediagram_H
