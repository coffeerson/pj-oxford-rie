#include "HardwareDiagram.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

// 颜色定义 (命名空间级别)
static const QColor C_NORMAL(200, 200, 200);
static const QColor C_RUNNING(100, 200, 100);
static const QColor C_WARNING(255, 200, 0);
static const QColor C_FAULT(255, 100, 100);
static const QColor C_PIPE(100, 100, 150);

// ==================== HardwareComponent ====================

HardwareComponent::HardwareComponent(const QString &name, ComponentType type, QGraphicsItem *parent)
    : QGraphicsItemGroup(parent)
    , m_name(name)
    , m_type(type)
    , m_state(ComponentState::Unknown)
    , m_value("")
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);

    // 名称标签
    m_nameText = new QGraphicsTextItem(this);
    m_nameText->setPlainText(name);
    m_nameText->setDefaultTextColor(Qt::white);
    m_nameText->setFont(QFont("Arial", 9, QFont::Bold));
    m_nameText->setPos(0, -20);

    // 数值标签
    m_valueText = new QGraphicsTextItem(this);
    m_valueText->setPlainText("");
    m_valueText->setDefaultTextColor(Qt::yellow);
    m_valueText->setFont(QFont("Arial", 8));
    m_valueText->setPos(0, 5);
}

void HardwareComponent::setState(ComponentState state)
{
    m_state = state;
    updateAppearance();
}

void HardwareComponent::setValue(const QString &value)
{
    m_value = value;
    m_valueText->setPlainText(value);
}

QRectF HardwareComponent::boundingRect() const
{
    return QRectF(0, -25, 80, 70);
}

// ==================== ChamberItem ====================

ChamberItem::ChamberItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Chamber, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void ChamberItem::updateAppearance()
{
    QColor color;
    switch (m_state) {
        case ComponentState::Running: color = C_RUNNING; break;
        case ComponentState::Warning: color = C_WARNING; break;
        case ComponentState::Fault: color = C_FAULT; break;
        default: color = C_NORMAL;
    }

    // 清除旧图形
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制腔室（椭圆形）
    QGraphicsEllipseItem *chamber = new QGraphicsEllipseItem(-35, -20, 70, 40, this);
    chamber->setBrush(QBrush(color));
    chamber->setPen(QPen(Qt::darkGray, 2));
    addToGroup(chamber);

    // 内部晶圆示意
    QGraphicsRectItem *wafer = new QGraphicsRectItem(-15, -8, 30, 16, this);
    wafer->setBrush(QBrush(Qt::darkBlue));
    wafer->setPen(QPen(Qt::blue, 1));
    addToGroup(wafer);
}

// ==================== RFGeneratorItem ====================

RFGeneratorItem::RFGeneratorItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::RFGenerator, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void RFGeneratorItem::updateAppearance()
{
    QColor color;
    switch (m_state) {
        case ComponentState::Running: color = C_RUNNING; break;
        case ComponentState::Warning: color = C_WARNING; break;
        case ComponentState::Fault: color = C_FAULT; break;
        default: color = C_NORMAL;
    }

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制 RF 发生器（矩形）
    QGraphicsRectItem *rf = new QGraphicsRectItem(-30, -20, 60, 40, this);
    rf->setBrush(QBrush(color));
    rf->setPen(QPen(Qt::darkGray, 2));
    addToGroup(rf);

    // RF 符号
    QGraphicsTextItem *rfLabel = new QGraphicsTextItem("13.56 MHz", this);
    rfLabel->setDefaultTextColor(Qt::white);
    rfLabel->setFont(QFont("Arial", 7));
    rfLabel->setPos(-20, -5);
    addToGroup(rfLabel);
}

// ==================== ICPGeneratorItem ====================

ICPGeneratorItem::ICPGeneratorItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::ICPGenerator, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void ICPGeneratorItem::updateAppearance()
{
    QColor color;
    switch (m_state) {
        case ComponentState::Running: color = C_RUNNING; break;
        case ComponentState::Warning: color = C_WARNING; break;
        case ComponentState::Fault: color = C_FAULT; break;
        default: color = C_NORMAL;
    }

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制 ICP 发生器（矩形）
    QGraphicsRectItem *icp = new QGraphicsRectItem(-30, -20, 60, 40, this);
    icp->setBrush(QBrush(color));
    icp->setPen(QPen(Qt::darkGray, 2));
    addToGroup(icp);

    // ICP 符号
    QGraphicsTextItem *icpLabel = new QGraphicsTextItem("2 MHz", this);
    icpLabel->setDefaultTextColor(Qt::white);
    icpLabel->setFont(QFont("Arial", 7));
    icpLabel->setPos(-15, -5);
    addToGroup(icpLabel);
}

// ==================== GasMFCItem ====================

GasMFCItem::GasMFCItem(const QString &name, int channel, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::GasMFC, parent)
    , m_channel(channel)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void GasMFCItem::updateAppearance()
{
    QColor color;
    switch (m_state) {
        case ComponentState::Running: color = C_RUNNING; break;
        case ComponentState::Warning: color = C_WARNING; break;
        case ComponentState::Fault: color = C_FAULT; break;
        default: color = C_NORMAL;
    }

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制 MFC（梯形效果用矩形）
    QGraphicsRectItem *mfc = new QGraphicsRectItem(-20, -15, 40, 30, this);
    mfc->setBrush(QBrush(color));
    mfc->setPen(QPen(Qt::darkGray, 2));
    addToGroup(mfc);

    // MFC 通道号
    QString chStr = QString("CH%1").arg(m_channel);
    QGraphicsTextItem *chLabel = new QGraphicsTextItem(chStr, this);
    chLabel->setDefaultTextColor(Qt::white);
    chLabel->setFont(QFont("Arial", 7, QFont::Bold));
    chLabel->setPos(-12, -5);
    addToGroup(chLabel);
}

// ==================== ValveItem ====================

ValveItem::ValveItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Valve, parent)
    , m_open(false)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void ValveItem::setOpen(bool open)
{
    m_open = open;
    updateAppearance();
}

void ValveItem::updateAppearance()
{
    QColor color = m_open ? C_RUNNING : C_NORMAL;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制阀门（三角形）
    QPainterPath path;
    path.moveTo(0, -12);
    path.lineTo(12, 8);
    path.lineTo(-12, 8);
    path.closeSubpath();

    QGraphicsPathItem *valve = new QGraphicsPathItem(path, this);
    valve->setBrush(QBrush(color));
    valve->setPen(QPen(Qt::darkGray, 2));
    addToGroup(valve);
}

// ==================== PumpItem ====================

PumpItem::PumpItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Pump, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void PumpItem::updateAppearance()
{
    QColor color;
    switch (m_state) {
        case ComponentState::Running: color = C_RUNNING; break;
        case ComponentState::Warning: color = C_WARNING; break;
        case ComponentState::Fault: color = C_FAULT; break;
        default: color = C_NORMAL;
    }

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制泵（圆形）
    QGraphicsEllipseItem *pump = new QGraphicsEllipseItem(-20, -20, 40, 40, this);
    pump->setBrush(QBrush(color));
    pump->setPen(QPen(Qt::darkGray, 2));
    addToGroup(pump);

    // 泵符号（P）
    QGraphicsTextItem *pLabel = new QGraphicsTextItem("P", this);
    pLabel->setDefaultTextColor(Qt::white);
    pLabel->setFont(QFont("Arial", 14, QFont::Bold));
    pLabel->setPos(-6, -10);
    addToGroup(pLabel);
}

// ==================== PressureGaugeItem ====================

PressureGaugeItem::PressureGaugeItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::PressureGauge, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void PressureGaugeItem::updateAppearance()
{
    QColor color;
    switch (m_state) {
        case ComponentState::Running: color = C_RUNNING; break;
        case ComponentState::Warning: color = C_WARNING; break;
        case ComponentState::Fault: color = C_FAULT; break;
        default: color = C_NORMAL;
    }

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制压力计（圆形仪表）
    QGraphicsEllipseItem *gauge = new QGraphicsEllipseItem(-18, -18, 36, 36, this);
    gauge->setBrush(QBrush(color));
    gauge->setPen(QPen(Qt::darkGray, 2));
    addToGroup(gauge);

    // 压力符号
    QGraphicsTextItem *pLabel = new QGraphicsTextItem("P", this);
    pLabel->setDefaultTextColor(Qt::white);
    pLabel->setFont(QFont("Arial", 10, QFont::Bold));
    pLabel->setPos(-4, -6);
    addToGroup(pLabel);
}

// ==================== ChuckItem ====================

ChuckItem::ChuckItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Chuck, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void ChuckItem::updateAppearance()
{
    QColor color;
    switch (m_state) {
        case ComponentState::Running: color = C_RUNNING; break;
        case ComponentState::Warning: color = C_WARNING; break;
        case ComponentState::Fault: color = C_FAULT; break;
        default: color = C_NORMAL;
    }

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制 Chuck（矩形）
    QGraphicsRectItem *chuck = new QGraphicsRectItem(-25, -12, 50, 24, this);
    chuck->setBrush(QBrush(color));
    chuck->setPen(QPen(Qt::darkGray, 2));
    addToGroup(chuck);

    // 温度符号
    QGraphicsTextItem *tLabel = new QGraphicsTextItem("°C", this);
    tLabel->setDefaultTextColor(Qt::white);
    tLabel->setFont(QFont("Arial", 7));
    tLabel->setPos(-8, -3);
    addToGroup(tLabel);
}

// ==================== PipeItem ====================

PipeItem::PipeItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Pipe, parent)
    , m_flowing(false)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void PipeItem::setFlowDirection(bool flowing)
{
    m_flowing = flowing;
    updateAppearance();
}

void PipeItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    QColor color = m_flowing ? C_RUNNING : C_PIPE;

    // 绘制管道（线条）- 子类或外部设置起点终点
    QGraphicsLineItem *pipe = new QGraphicsLineItem(-30, 0, 30, 0, this);
    pipe->setPen(QPen(color, 6));
    addToGroup(pipe);
}

// ==================== HardwareDiagram ====================

HardwareDiagram::HardwareDiagram(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setBackgroundBrush(QBrush(QColor(30, 30, 40)));

    // 设置场景大小
    m_scene->setSceneRect(0, 0, 900, 600);

    setupLayout();
}

HardwareDiagram::~HardwareDiagram()
{
}

void HardwareDiagram::setupLayout()
{
    m_scene->clear();
    m_components.clear();

    // 背景网格
    for (int x = 0; x < 900; x += 50) {
        m_scene->addLine(x, 0, x, 600, QPen(QColor(50, 50, 60), 1));
    }
    for (int y = 0; y < 600; y += 50) {
        m_scene->addLine(0, y, 900, y, QPen(QColor(50, 50, 60), 1));
    }

    // 标题
    QGraphicsTextItem *title = m_scene->addText(
        QStringLiteral("Oxford ICP133 RIE 设备硬件框架图"),
        QFont("Arial", 16, QFont::Bold));
    title->setDefaultTextColor(Qt::white);
    title->setPos(300, 10);

    // ===== 布局组件 =====
    // 中心腔室
    ChamberItem *chamber = new ChamberItem(QStringLiteral("真空腔室"));
    chamber->setPos(450, 280);
    chamber->setState(ComponentState::Normal);
    m_scene->addItem(chamber);
    m_components[QStringLiteral("Chamber")] = chamber;

    // Chuck (在腔室下方)
    ChuckItem *chuck = new ChuckItem(QStringLiteral("Chuck"));
    chuck->setPos(450, 380);
    chuck->setState(ComponentState::Normal);
    m_scene->addItem(chuck);
    m_components[QStringLiteral("Chuck")] = chuck;

    // RF 发生器 (腔室左侧)
    RFGeneratorItem *rf = new RFGeneratorItem(QStringLiteral("RF Bias"));
    rf->setPos(250, 280);
    rf->setState(ComponentState::Normal);
    m_scene->addItem(rf);
    m_components[QStringLiteral("RF")] = rf;

    // ICP 发生器 (腔室上方)
    ICPGeneratorItem *icp = new ICPGeneratorItem(QStringLiteral("ICP Source"));
    icp->setPos(450, 150);
    icp->setState(ComponentState::Normal);
    m_scene->addItem(icp);
    m_components[QStringLiteral("ICP")] = icp;

    // 压力计 (腔室右侧)
    PressureGaugeItem *pressure = new PressureGaugeItem(QStringLiteral("压力计"));
    pressure->setPos(620, 280);
    pressure->setState(ComponentState::Normal);
    m_scene->addItem(pressure);
    m_components[QStringLiteral("Pressure")] = pressure;

    // 机械泵 (底部左侧)
    PumpItem *mp = new PumpItem(QStringLiteral("机械泵"));
    mp->setPos(150, 450);
    mp->setState(ComponentState::Normal);
    m_scene->addItem(mp);
    m_components[QStringLiteral("MP")] = mp;

    // 分子泵 (底部右侧)
    PumpItem *tp = new PumpItem(QStringLiteral("分子泵"));
    tp->setPos(750, 450);
    tp->setState(ComponentState::Normal);
    m_scene->addItem(tp);
    m_components[QStringLiteral("TP")] = tp;

    // 气体 MFC (左侧)
    GasMFCItem *mfc1 = new GasMFCItem(QStringLiteral("MFC1"), 1);
    mfc1->setPos(100, 180);
    mfc1->setState(ComponentState::Normal);
    m_scene->addItem(mfc1);
    m_components[QStringLiteral("MFC1")] = mfc1;

    GasMFCItem *mfc2 = new GasMFCItem(QStringLiteral("MFC2"), 2);
    mfc2->setPos(100, 240);
    mfc2->setState(ComponentState::Normal);
    m_scene->addItem(mfc2);
    m_components[QStringLiteral("MFC2")] = mfc2;

    GasMFCItem *mfc3 = new GasMFCItem(QStringLiteral("MFC3"), 3);
    mfc3->setPos(100, 300);
    mfc3->setState(ComponentState::Normal);
    m_scene->addItem(mfc3);
    m_components[QStringLiteral("MFC3")] = mfc3;

    GasMFCItem *mfc4 = new GasMFCItem(QStringLiteral("MFC4"), 4);
    mfc4->setPos(100, 360);
    mfc4->setState(ComponentState::Normal);
    m_scene->addItem(mfc4);
    m_components[QStringLiteral("MFC4")] = mfc4;

    // 阀门
    ValveItem *v1 = new ValveItem(QStringLiteral("V1"));
    v1->setPos(200, 280);
    v1->setOpen(false);
    m_scene->addItem(v1);
    m_components[QStringLiteral("V1")] = v1;

    ValveItem *v2 = new ValveItem(QStringLiteral("V2"));
    v2->setPos(350, 280);
    v2->setOpen(false);
    m_scene->addItem(v2);
    m_components[QStringLiteral("V2")] = v2;

    ValveItem *v3 = new ValveItem(QStringLiteral("V3"));
    v3->setPos(550, 280);
    v3->setOpen(false);
    m_scene->addItem(v3);
    m_components[QStringLiteral("V3")] = v3;

    ValveItem *v4 = new ValveItem(QStringLiteral("V4"));
    v4->setPos(620, 350);
    v4->setOpen(false);
    m_scene->addItem(v4);
    m_components[QStringLiteral("V4")] = v4;

    ValveItem *v5 = new ValveItem(QStringLiteral("V5"));
    v5->setPos(350, 450);
    v5->setOpen(false);
    m_scene->addItem(v5);
    m_components[QStringLiteral("V5")] = v5;

    ValveItem *v6 = new ValveItem(QStringLiteral("V6"));
    v6->setPos(550, 450);
    v6->setOpen(false);
    m_scene->addItem(v6);
    m_components[QStringLiteral("V6")] = v6;

    // ===== 绘制管道连接 =====
    // MFC1 -> V1 -> Chamber
    m_scene->addLine(140, 180, 200, 180, QPen(C_PIPE, 4));
    m_scene->addLine(200, 180, 200, 280, QPen(C_PIPE, 4));

    // MFC2 -> V1
    m_scene->addLine(140, 240, 200, 240, QPen(C_PIPE, 4));

    // MFC3 -> V2 -> Chamber
    m_scene->addLine(140, 300, 170, 300, QPen(C_PIPE, 4));
    m_scene->addLine(170, 300, 170, 280, QPen(C_PIPE, 4));
    m_scene->addLine(170, 280, 350, 280, QPen(C_PIPE, 4));

    // MFC4 -> V2
    m_scene->addLine(140, 360, 170, 360, QPen(C_PIPE, 4));
    m_scene->addLine(170, 360, 170, 300, QPen(C_PIPE, 4));

    // Chamber -> V3 -> Pressure
    m_scene->addLine(515, 280, 550, 280, QPen(C_PIPE, 4));

    // Chamber -> V4 (向下)
    m_scene->addLine(550, 300, 620, 300, QPen(C_PIPE, 4));
    m_scene->addLine(620, 300, 620, 350, QPen(C_PIPE, 4));

    // Pressure -> V4 -> 分子泵
    m_scene->addLine(638, 280, 700, 280, QPen(C_PIPE, 4));
    m_scene->addLine(700, 280, 700, 350, QPen(C_PIPE, 4));
    m_scene->addLine(700, 350, 730, 350, QPen(C_PIPE, 4));
    m_scene->addLine(730, 350, 730, 430, QPen(C_PIPE, 4));

    // V5 -> 机械泵
    m_scene->addLine(350, 320, 350, 430, QPen(C_PIPE, 4));
    m_scene->addLine(350, 430, 170, 430, QPen(C_PIPE, 4));
    m_scene->addLine(170, 430, 170, 430, QPen(C_PIPE, 4));

    // V6 -> 分子泵
    m_scene->addLine(550, 320, 550, 430, QPen(C_PIPE, 4));
    m_scene->addLine(550, 430, 730, 430, QPen(C_PIPE, 4));

    // ICP -> Chamber (虚线)
    m_scene->addLine(450, 190, 450, 240, QPen(C_PIPE, 3, Qt::DashLine));

    // RF -> Chamber (虚线)
    m_scene->addLine(280, 280, 415, 280, QPen(C_PIPE, 3, Qt::DashLine));

    // Chuck -> Chamber
    m_scene->addLine(450, 320, 450, 368, QPen(C_PIPE, 4));

    // ===== 图例 =====
    QGraphicsTextItem *legend = m_scene->addText(QStringLiteral("图例:"), QFont("Arial", 10, QFont::Bold));
    legend->setDefaultTextColor(Qt::white);
    legend->setPos(20, 520);

    // 运行状态示例
    QGraphicsRectItem *runSample = m_scene->addRect(70, 515, 15, 15);
    runSample->setBrush(QBrush(C_RUNNING));
    m_scene->addText(QStringLiteral("运行"), QFont("Arial", 8))->setPos(90, 515);

    QGraphicsRectItem *stopSample = m_scene->addRect(140, 515, 15, 15);
    stopSample->setBrush(QBrush(C_NORMAL));
    m_scene->addText(QStringLiteral("停止"), QFont("Arial", 8))->setPos(160, 515);

    QGraphicsRectItem *warnSample = m_scene->addRect(210, 515, 15, 15);
    warnSample->setBrush(QBrush(C_WARNING));
    m_scene->addText(QStringLiteral("警告"), QFont("Arial", 8))->setPos(230, 515);

    QGraphicsRectItem *faultSample = m_scene->addRect(280, 515, 15, 15);
    faultSample->setBrush(QBrush(C_FAULT));
    m_scene->addText(QStringLiteral("故障"), QFont("Arial", 8))->setPos(300, 515);
}

void HardwareDiagram::updateComponentState(const QString &name, ComponentState state)
{
    HardwareComponent *comp = m_components.value(name, nullptr);
    if (comp) {
        comp->setState(state);
    }
}

void HardwareDiagram::updateComponentValue(const QString &name, const QString &value)
{
    HardwareComponent *comp = m_components.value(name, nullptr);
    if (comp) {
        comp->setValue(value);
    }
}

void HardwareDiagram::setValveOpen(const QString &name, bool open)
{
    ValveItem *valve = dynamic_cast<ValveItem*>(m_components.value(name, nullptr));
    if (valve) {
        valve->setOpen(open);
    }
}

void HardwareDiagram::setPipeFlow(const QString &name, bool flowing)
{
    Q_UNUSED(name);
    Q_UNUSED(flowing);
    // 管道流动状态可以通过颜色变化实现
}

HardwareComponent* HardwareDiagram::getComponent(const QString &name) const
{
    return m_components.value(name, nullptr);
}

void HardwareDiagram::onItemClicked(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    // 处理组件点击事件
    HardwareComponent *item = dynamic_cast<HardwareComponent*>(sender());
    if (item) {
        emit componentClicked(item->getName(), item->getType());
    }
}
