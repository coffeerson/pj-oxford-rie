#include "HardwareDiagram.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGradient>

// Oxford 原厂风格配色
static const QColor C_BG_DARK(QColor(50, 56, 63));        // 深蓝灰背景
static const QColor C_BG_PANEL(QColor(45, 50, 56));        // 面板背景
static const QColor C_LED_GREEN(QColor(0, 200, 0));         // 绿色 LED
static const QColor C_LED_RED(QColor(220, 50, 50));        // 红色 LED
static const QColor C_LED_YELLOW(QColor(255, 200, 0));     // 黄色 LED
static const QColor C_LED_OFF(QColor(80, 80, 80));        // LED 熄灭
static const QColor C_COMPONENT_ON(QColor(0, 180, 0));      // 组件开启
static const QColor C_COMPONENT_OFF(QColor(100, 100, 100)); // 组件关闭
static const QColor C_PIPE(QColor(80, 80, 120));           // 管道
static const QColor C_TEXT_WHITE(Qt::white);
static const QColor C_TEXT_YELLOW(Qt::yellow);

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
    m_nameText->setDefaultTextColor(C_TEXT_WHITE);
    m_nameText->setFont(QFont("Arial", 8, QFont::Bold));
    m_nameText->setPos(0, -18);

    // 数值标签
    m_valueText = new QGraphicsTextItem(this);
    m_valueText->setPlainText("");
    m_valueText->setDefaultTextColor(C_TEXT_YELLOW);
    m_valueText->setFont(QFont("Courier New", 9));
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
    return QRectF(0, -22, 70, 60);
}

// ==================== ChamberItem ====================

ChamberItem::ChamberItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Chamber, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void ChamberItem::updateAppearance()
{
    QColor color = (m_state == ComponentState::Running) ? C_COMPONENT_ON : C_COMPONENT_OFF;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制腔室（圆角矩形，类似原厂）
    QGraphicsRectItem *chamber = new QGraphicsRectItem(-40, -25, 80, 50, this);
    chamber->setBrush(QBrush(color));
    chamber->setPen(QPen(Qt::darkGray, 2));
    addToGroup(chamber);

    // 内部晶圆示意（椭圆形）
    QGraphicsEllipseItem *wafer = new QGraphicsEllipseItem(-20, -12, 40, 24, this);
    wafer->setBrush(QBrush(Qt::darkBlue));
    wafer->setPen(QPen(Qt::blue, 1));
    addToGroup(wafer);
}

// ==================== LEDIndicator ====================

// 独立 LED 指示灯组件
class LEDIndicator : public QGraphicsItemGroup
{
public:
    LEDIndicator(bool on, const QColor &onColor, const QColor &offColor, QGraphicsItem *parent = nullptr)
        : QGraphicsItemGroup(parent)
        , m_on(on)
        , m_onColor(onColor)
        , m_offColor(offColor)
    {
        updateAppearance();
    }

    void setOn(bool on) {
        m_on = on;
        updateAppearance();
    }

private:
    void updateAppearance() {
        for (auto child : childItems()) {
            removeFromGroup(child);
            delete child;
        }

        QColor color = m_on ? m_onColor : m_offColor;

        // LED 外圈
        QGraphicsEllipseItem *led = new QGraphicsEllipseItem(-6, -6, 12, 12, this);
        led->setBrush(QBrush(color));
        led->setPen(QPen(Qt::darkGray, 1));
        addToGroup(led);

        // LED 高光
        QGraphicsEllipseItem *highlight = new QGraphicsEllipseItem(-4, -4, 4, 4, this);
        highlight->setBrush(QBrush(Qt::white));
        highlight->setPen(Qt::NoPen);
        addToGroup(highlight);
    }

    bool m_on;
    QColor m_onColor;
    QColor m_offColor;
};

// ==================== RFGeneratorItem ====================

RFGeneratorItem::RFGeneratorItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::RFGenerator, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void RFGeneratorItem::updateAppearance()
{
    QColor color = (m_state == ComponentState::Running) ? C_COMPONENT_ON : C_COMPONENT_OFF;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制 RF 发生器（矩形）
    QGraphicsRectItem *rf = new QGraphicsRectItem(-35, -22, 70, 44, this);
    rf->setBrush(QBrush(color));
    rf->setPen(QPen(Qt::darkGray, 2));
    addToGroup(rf);

    // RF 频率标签
    QGraphicsTextItem *rfLabel = new QGraphicsTextItem("13.56", this);
    rfLabel->setDefaultTextColor(Qt::white);
    rfLabel->setFont(QFont("Arial", 7));
    rfLabel->setPos(-18, -8);
    addToGroup(rfLabel);

    QGraphicsTextItem *mhzLabel = new QGraphicsTextItem("MHz", this);
    mhzLabel->setDefaultTextColor(Qt::white);
    mhzLabel->setFont(QFont("Arial", 6));
    mhzLabel->setPos(-12, 2);
    addToGroup(mhzLabel);
}

// ==================== ICPGeneratorItem ====================

ICPGeneratorItem::ICPGeneratorItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::ICPGenerator, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void ICPGeneratorItem::updateAppearance()
{
    QColor color = (m_state == ComponentState::Running) ? C_COMPONENT_ON : C_COMPONENT_OFF;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制 ICP 发生器（矩形）
    QGraphicsRectItem *icp = new QGraphicsRectItem(-35, -22, 70, 44, this);
    icp->setBrush(QBrush(color));
    icp->setPen(QPen(Qt::darkGray, 2));
    addToGroup(icp);

    // ICP 频率标签
    QGraphicsTextItem *icpLabel = new QGraphicsTextItem("2", this);
    icpLabel->setDefaultTextColor(Qt::white);
    icpLabel->setFont(QFont("Arial", 10, QFont::Bold));
    icpLabel->setPos(-5, -10);
    addToGroup(icpLabel);

    QGraphicsTextItem *mhzLabel = new QGraphicsTextItem("MHz", this);
    mhzLabel->setDefaultTextColor(Qt::white);
    mhzLabel->setFont(QFont("Arial", 6));
    mhzLabel->setPos(-10, 2);
    addToGroup(mhzLabel);
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
    QColor color = (m_state == ComponentState::Running) ? C_COMPONENT_ON : C_COMPONENT_OFF;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制 MFC（矩形）
    QGraphicsRectItem *mfc = new QGraphicsRectItem(-25, -18, 50, 36, this);
    mfc->setBrush(QBrush(color));
    mfc->setPen(QPen(Qt::darkGray, 2));
    addToGroup(mfc);

    // MFC 通道号
    QString chStr = QString("CH%1").arg(m_channel);
    QGraphicsTextItem *chLabel = new QGraphicsTextItem(chStr, this);
    chLabel->setDefaultTextColor(Qt::white);
    chLabel->setFont(QFont("Arial", 8, QFont::Bold));
    chLabel->setPos(-14, -6);
    addToGroup(chLabel);

    // MFC 图标（小矩形表示流量）
    QGraphicsRectItem *flow = new QGraphicsRectItem(-8, 6, 16, 6, this);
    flow->setBrush(QBrush(Qt::darkGreen));
    flow->setPen(QPen(Qt::darkGray, 1));
    addToGroup(flow);
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
    // 原厂风格：绿色=开启，红色=关闭
    QColor color = m_open ? C_LED_GREEN : C_LED_RED;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制阀门（菱形/钻石形状，类似原厂 schematic）
    QPainterPath path;
    path.moveTo(0, -14);
    path.lineTo(14, 0);
    path.lineTo(0, 14);
    path.lineTo(-14, 0);
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
    QColor color = (m_state == ComponentState::Running) ? C_COMPONENT_ON : C_COMPONENT_OFF;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制泵（圆形）
    QGraphicsEllipseItem *pump = new QGraphicsEllipseItem(-22, -22, 44, 44, this);
    pump->setBrush(QBrush(color));
    pump->setPen(QPen(Qt::darkGray, 2));
    addToGroup(pump);

    // 泵符号（P）
    QGraphicsTextItem *pLabel = new QGraphicsTextItem("P", this);
    pLabel->setDefaultTextColor(Qt::white);
    pLabel->setFont(QFont("Arial", 16, QFont::Bold));
    pLabel->setPos(-7, -12);
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
    QColor color = (m_state == ComponentState::Running) ? C_COMPONENT_ON : C_COMPONENT_OFF;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制压力计（圆形仪表）
    QGraphicsEllipseItem *gauge = new QGraphicsEllipseItem(-20, -20, 40, 40, this);
    gauge->setBrush(QBrush(color));
    gauge->setPen(QPen(Qt::darkGray, 2));
    addToGroup(gauge);

    // 压力符号
    QGraphicsTextItem *pLabel = new QGraphicsTextItem("P", this);
    pLabel->setDefaultTextColor(Qt::white);
    pLabel->setFont(QFont("Arial", 12, QFont::Bold));
    pLabel->setPos(-5, -8);
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
    QColor color = (m_state == ComponentState::Running) ? C_COMPONENT_ON : C_COMPONENT_OFF;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 绘制 Chuck（矩形）
    QGraphicsRectItem *chuck = new QGraphicsRectItem(-30, -15, 60, 30, this);
    chuck->setBrush(QBrush(color));
    chuck->setPen(QPen(Qt::darkGray, 2));
    addToGroup(chuck);

    // 温度符号
    QGraphicsTextItem *tLabel = new QGraphicsTextItem("°C", this);
    tLabel->setDefaultTextColor(Qt::white);
    tLabel->setFont(QFont("Arial", 8));
    tLabel->setPos(-10, -4);
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

    QColor color = m_flowing ? C_COMPONENT_ON : C_PIPE;

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
    // Oxford 原厂深色背景
    setBackgroundBrush(QBrush(C_BG_DARK));

    // 设置场景大小
    m_scene->setSceneRect(0, 0, 1000, 700);

    setupLayout();
}

HardwareDiagram::~HardwareDiagram()
{
}

void HardwareDiagram::setupLayout()
{
    m_scene->clear();
    m_components.clear();

    // ===== 标题栏 =====
    QGraphicsRectItem *titleBar = new QGraphicsRectItem(0, 0, 1000, 40);
    titleBar->setBrush(QBrush(QColor(30, 35, 40)));
    titleBar->setPen(Qt::NoPen);
    m_scene->addItem(titleBar);

    QGraphicsTextItem *title = m_scene->addText(
        QStringLiteral("Oxford ICP133 RIE - Plasmalab System133"),
        QFont("Arial", 14, QFont::Bold));
    title->setDefaultTextColor(Qt::white);
    title->setPos(20, 12);

    // 状态指示
    QGraphicsTextItem *statusText = m_scene->addText(
        QStringLiteral("System Status: STANDBY"),
        QFont("Arial", 10));
    statusText->setDefaultTextColor(C_LED_YELLOW);
    statusText->setPos(400, 14);

    // STOP 按钮
    QGraphicsRectItem *stopBtn = new QGraphicsRectItem(880, 8, 100, 26);
    stopBtn->setBrush(QBrush(C_LED_RED));
    stopBtn->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(stopBtn);

    QGraphicsTextItem *stopLabel = m_scene->addText("STOP ALL", QFont("Arial", 9, QFont::Bold));
    stopLabel->setDefaultTextColor(Qt::white);
    stopLabel->setPos(890, 14);

    // ===== 硬件示意图区域 =====
    // 中心腔室
    ChamberItem *chamber = new ChamberItem(QStringLiteral("Chamber"));
    chamber->setPos(500, 300);
    chamber->setState(ComponentState::Normal);
    m_scene->addItem(chamber);
    m_components[QStringLiteral("Chamber")] = chamber;

    // Chuck
    ChuckItem *chuck = new ChuckItem(QStringLiteral("Chuck"));
    chuck->setPos(500, 420);
    chuck->setState(ComponentState::Normal);
    m_scene->addItem(chuck);
    m_components[QStringLiteral("Chuck")] = chuck;

    // RF 发生器
    RFGeneratorItem *rf = new RFGeneratorItem(QStringLiteral("RF"));
    rf->setPos(280, 300);
    rf->setState(ComponentState::Normal);
    m_scene->addItem(rf);
    m_components[QStringLiteral("RF")] = rf;

    // ICP 发生器
    ICPGeneratorItem *icp = new ICPGeneratorItem(QStringLiteral("ICP"));
    icp->setPos(500, 130);
    icp->setState(ComponentState::Normal);
    m_scene->addItem(icp);
    m_components[QStringLiteral("ICP")] = icp;

    // 压力计
    PressureGaugeItem *pressure = new PressureGaugeItem(QStringLiteral("Pressure"));
    pressure->setPos(720, 300);
    pressure->setState(ComponentState::Normal);
    m_scene->addItem(pressure);
    m_components[QStringLiteral("Pressure")] = pressure;

    // 机械泵
    PumpItem *mp = new PumpItem(QStringLiteral("MP"));
    mp->setPos(200, 500);
    mp->setState(ComponentState::Normal);
    m_scene->addItem(mp);
    m_components[QStringLiteral("MP")] = mp;

    // 分子泵
    PumpItem *tp = new PumpItem(QStringLiteral("TP"));
    tp->setPos(800, 500);
    tp->setState(ComponentState::Normal);
    m_scene->addItem(tp);
    m_components[QStringLiteral("TP")] = tp;

    // 气体 MFC
    GasMFCItem *mfc1 = new GasMFCItem(QStringLiteral("MFC1"), 1);
    mfc1->setPos(100, 160);
    mfc1->setState(ComponentState::Normal);
    m_scene->addItem(mfc1);
    m_components[QStringLiteral("MFC1")] = mfc1;

    GasMFCItem *mfc2 = new GasMFCItem(QStringLiteral("MFC2"), 2);
    mfc2->setPos(100, 240);
    mfc2->setState(ComponentState::Normal);
    m_scene->addItem(mfc2);
    m_components[QStringLiteral("MFC2")] = mfc2;

    GasMFCItem *mfc3 = new GasMFCItem(QStringLiteral("MFC3"), 3);
    mfc3->setPos(100, 320);
    mfc3->setState(ComponentState::Normal);
    m_scene->addItem(mfc3);
    m_components[QStringLiteral("MFC3")] = mfc3;

    GasMFCItem *mfc4 = new GasMFCItem(QStringLiteral("MFC4"), 4);
    mfc4->setPos(100, 400);
    mfc4->setState(ComponentState::Normal);
    m_scene->addItem(mfc4);
    m_components[QStringLiteral("MFC4")] = mfc4;

    // 阀门
    ValveItem *v1 = new ValveItem(QStringLiteral("V1"));
    v1->setPos(200, 300);
    v1->setOpen(false);
    m_scene->addItem(v1);
    m_components[QStringLiteral("V1")] = v1;

    ValveItem *v2 = new ValveItem(QStringLiteral("V2"));
    v2->setPos(380, 300);
    v2->setOpen(false);
    m_scene->addItem(v2);
    m_components[QStringLiteral("V2")] = v2;

    ValveItem *v3 = new ValveItem(QStringLiteral("V3"));
    v3->setPos(620, 300);
    v3->setOpen(false);
    m_scene->addItem(v3);
    m_components[QStringLiteral("V3")] = v3;

    ValveItem *v4 = new ValveItem(QStringLiteral("V4"));
    v4->setPos(720, 380);
    v4->setOpen(false);
    m_scene->addItem(v4);
    m_components[QStringLiteral("V4")] = v4;

    ValveItem *v5 = new ValveItem(QStringLiteral("V5"));
    v5->setPos(380, 500);
    v5->setOpen(false);
    m_scene->addItem(v5);
    m_components[QStringLiteral("V5")] = v5;

    ValveItem *v6 = new ValveItem(QStringLiteral("V6"));
    v6->setPos(620, 500);
    v6->setOpen(false);
    m_scene->addItem(v6);
    m_components[QStringLiteral("V6")] = v6;

    // ===== 绘制管道连接 =====
    // 气体管道路由
    m_scene->addLine(150, 160, 200, 160, QPen(C_PIPE, 4));
    m_scene->addLine(200, 160, 200, 300, QPen(C_PIPE, 4));

    m_scene->addLine(150, 240, 200, 240, QPen(C_PIPE, 4));

    m_scene->addLine(150, 320, 200, 320, QPen(C_PIPE, 4));
    m_scene->addLine(200, 320, 200, 300, QPen(C_PIPE, 4));

    m_scene->addLine(150, 400, 200, 400, QPen(C_PIPE, 4));

    // RF -> Chamber
    m_scene->addLine(315, 300, 460, 300, QPen(C_PIPE, 3, Qt::DashLine));

    // ICP -> Chamber
    m_scene->addLine(500, 174, 500, 250, QPen(C_PIPE, 3, Qt::DashLine));

    // Chamber -> V3 -> Pressure
    m_scene->addLine(540, 300, 620, 300, QPen(C_PIPE, 4));

    // Chamber -> V4 -> TP
    m_scene->addLine(560, 320, 720, 320, QPen(C_PIPE, 4));
    m_scene->addLine(720, 320, 720, 380, QPen(C_PIPE, 4));
    m_scene->addLine(720, 380, 778, 380, QPen(C_PIPE, 4));

    // Pressure -> V4
    m_scene->addLine(740, 300, 740, 320, QPen(C_PIPE, 4));

    // V5 -> MP
    m_scene->addLine(380, 340, 380, 478, QPen(C_PIPE, 4));
    m_scene->addLine(380, 478, 178, 478, QPen(C_PIPE, 4));

    // V6 -> TP
    m_scene->addLine(620, 340, 620, 478, QPen(C_PIPE, 4));
    m_scene->addLine(620, 478, 778, 478, QPen(C_PIPE, 4));

    // Chuck -> Chamber
    m_scene->addLine(500, 345, 500, 405, QPen(C_PIPE, 4));

    // ===== 底部状态面板 =====
    QGraphicsRectItem *panel = new QGraphicsRectItem(0, 540, 1000, 160);
    panel->setBrush(QBrush(C_BG_PANEL));
    panel->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(panel);

    // 面板标题
    QGraphicsTextItem *panelTitle = m_scene->addText(
        QStringLiteral("PROCESS STATUS"),
        QFont("Arial", 11, QFont::Bold));
    panelTitle->setDefaultTextColor(Qt::white);
    panelTitle->setPos(20, 555);

    // 状态 LED 指示灯
    // Process Status
    QGraphicsTextItem *procLabel = m_scene->addText("Process:", QFont("Arial", 9));
    procLabel->setDefaultTextColor(Qt::white);
    procLabel->setPos(20, 585);

    // 绿色 LED 表示 ACTIVE
    LEDIndicator *procLed = new LEDIndicator(true, C_LED_GREEN, C_LED_OFF);
    procLed->setPos(100, 582);
    m_scene->addItem(procLed);

    QGraphicsTextItem *procStatus = m_scene->addText("READY", QFont("Arial", 9, QFont::Bold));
    procStatus->setDefaultTextColor(C_LED_GREEN);
    procStatus->setPos(115, 585);

    // RF Status
    QGraphicsTextItem *rfLabel = m_scene->addText("RF:", QFont("Arial", 9));
    rfLabel->setDefaultTextColor(Qt::white);
    rfLabel->setPos(200, 585);

    LEDIndicator *rfLed = new LEDIndicator(false, C_LED_GREEN, C_LED_OFF);
    rfLed->setPos(235, 582);
    m_scene->addItem(rfLed);

    QGraphicsTextItem *rfStatus = m_scene->addText("OFF", QFont("Arial", 9, QFont::Bold));
    rfStatus->setDefaultTextColor(C_LED_OFF);
    rfStatus->setPos(250, 585);

    // ICP Status
    QGraphicsTextItem *icpLabel = m_scene->addText("ICP:", QFont("Arial", 9));
    icpLabel->setDefaultTextColor(Qt::white);
    icpLabel->setPos(300, 585);

    LEDIndicator *icpLed = new LEDIndicator(false, C_LED_GREEN, C_LED_OFF);
    icpLed->setPos(340, 582);
    m_scene->addItem(icpLed);

    QGraphicsTextItem *icpStatus = m_scene->addText("OFF", QFont("Arial", 9, QFont::Bold));
    icpStatus->setDefaultTextColor(C_LED_OFF);
    icpStatus->setPos(355, 585);

    // Pressure 显示
    QGraphicsTextItem *presLabel = m_scene->addText("Pressure:", QFont("Arial", 9));
    presLabel->setDefaultTextColor(Qt::white);
    presLabel->setPos(420, 585);

    QGraphicsTextItem *presValue = m_scene->addText("0.00 mbar", QFont("Courier New", 10, QFont::Bold));
    presValue->setDefaultTextColor(C_TEXT_YELLOW);
    presValue->setPos(490, 585);

    // Temperature 显示
    QGraphicsTextItem *tempLabel = m_scene->addText("Temp:", QFont("Arial", 9));
    tempLabel->setDefaultTextColor(Qt::white);
    tempLabel->setPos(600, 585);

    QGraphicsTextItem *tempValue = m_scene->addText("21.5 °C", QFont("Courier New", 10, QFont::Bold));
    tempValue->setDefaultTextColor(C_TEXT_YELLOW);
    tempValue->setPos(650, 585);

    // Gas Flow 显示
    QGraphicsTextItem *gasLabel = m_scene->addText("Gas Flow:", QFont("Arial", 9));
    gasLabel->setDefaultTextColor(Qt::white);
    gasLabel->setPos(750, 585);

    QGraphicsTextItem *gasValue = m_scene->addText("0.0 sccm", QFont("Courier New", 10, QFont::Bold));
    gasValue->setDefaultTextColor(C_TEXT_YELLOW);
    gasValue->setPos(830, 585);

    // 控制按钮
    QGraphicsRectItem *evacBtn = new QGraphicsRectItem(20, 620, 90, 30);
    evacBtn->setBrush(QBrush(QColor(0, 120, 0)));
    evacBtn->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(evacBtn);
    QGraphicsTextItem *evacLabel = m_scene->addText("EVACUATE", QFont("Arial", 9, QFont::Bold));
    evacLabel->setDefaultTextColor(Qt::white);
    evacLabel->setPos(28, 628);

    QGraphicsRectItem *stopBtn2 = new QGraphicsRectItem(130, 620, 90, 30);
    stopBtn2->setBrush(QBrush(C_LED_RED));
    stopBtn2->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(stopBtn2);
    QGraphicsTextItem *stopLabel2 = m_scene->addText("STOP", QFont("Arial", 9, QFont::Bold));
    stopLabel2->setDefaultTextColor(Qt::white);
    stopLabel2->setPos(155, 628);

    QGraphicsRectItem *ventBtn = new QGraphicsRectItem(240, 620, 90, 30);
    ventBtn->setBrush(QBrush(QColor(100, 100, 100)));
    ventBtn->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(ventBtn);
    QGraphicsTextItem *ventLabel = m_scene->addText("VENT", QFont("Arial", 9, QFont::Bold));
    ventLabel->setDefaultTextColor(Qt::white);
    ventLabel->setPos(265, 628);

    // ===== 图例 =====
    QGraphicsTextItem *legendTitle = m_scene->addText(QStringLiteral("Legend:"), QFont("Arial", 9, QFont::Bold));
    legendTitle->setDefaultTextColor(Qt::white);
    legendTitle->setPos(850, 555);

    // 阀门图例
    LEDIndicator *valveOpen = new LEDIndicator(true, C_LED_GREEN, C_LED_OFF);
    valveOpen->setPos(850, 582);
    m_scene->addItem(valveOpen);
    QGraphicsTextItem *valveOpenLabel = m_scene->addText("Valve Open", QFont("Arial", 8));
    valveOpenLabel->setDefaultTextColor(Qt::white);
    valveOpenLabel->setPos(865, 585);

    LEDIndicator *valveClosed = new LEDIndicator(true, C_LED_RED, C_LED_OFF);
    valveClosed->setPos(850, 608);
    m_scene->addItem(valveClosed);
    QGraphicsTextItem *valveClosedLabel = m_scene->addText("Valve Closed", QFont("Arial", 8));
    valveClosedLabel->setDefaultTextColor(Qt::white);
    valveClosedLabel->setPos(865, 608);
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
}

HardwareComponent* HardwareDiagram::getComponent(const QString &name) const
{
    return m_components.value(name, nullptr);
}

void HardwareDiagram::onItemClicked(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    HardwareComponent *item = dynamic_cast<HardwareComponent*>(sender());
    if (item) {
        emit componentClicked(item->getName(), item->getType());
    }
}
