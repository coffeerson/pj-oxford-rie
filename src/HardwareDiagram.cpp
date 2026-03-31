#include "HardwareDiagram.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

// Oxford PC 2000 原厂配色
static const QColor C_WIN_GRAY(192, 192, 192);       // Windows 经典灰背景
static const QColor C_WIN_DARK(128, 128, 128);       // 深灰
static const QColor C_WIN_MEDIUM(160, 160, 160);     // 中灰
static const QColor C_WIN_LIGHT(224, 224, 224);       // 浅灰
static const QColor C_TITLE_BLUE(0, 0, 128);          // 标题栏深蓝
static const QColor C_TITLE_BLUE_LIGHT(24, 24, 240); // 标题栏浅蓝
static const QColor C_LED_GREEN(0, 255, 0);           // LED 亮绿
static const QColor C_LED_GREEN_DARK(0, 64, 0);      // LED 暗绿
static const QColor C_LED_RED(255, 0, 0);            // LED 红
static const QColor C_LED_RED_DARK(64, 0, 0);        // LED 暗红
static const QColor C_LED_BLUE(0, 0, 255);           // LED 蓝
static const QColor C_PIPE_ACTIVE(0, 0, 200);        // 管道激活（蓝）
static const QColor C_PIPE_INACTIVE(64, 64, 64);      // 管道非激活
static const QColor C_CHAMBER_OUTER(100, 100, 100);  // 腔室外圈
static const QColor C_CHAMBER_INNER(160, 160, 160);  // 腔室内圈
static const QColor C_CHAMBER_CENTER(40, 40, 40);    // 腔室中心
static const QColor C_BELLOW(80, 80, 80);            // 波纹管

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

    m_nameText = new QGraphicsTextItem(this);
    m_nameText->setPlainText(name);
    m_nameText->setDefaultTextColor(Qt::black);
    m_nameText->setFont(QFont("Arial", 7));
    m_nameText->setPos(0, -18);
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
    return QRectF(0, -20, 60, 55);
}

// ==================== ChamberItem ====================
// 腔室：大圆形，同心圆结构，十字准星中心

ChamberItem::ChamberItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Chamber, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void ChamberItem::updateAppearance()
{
    QColor activeColor = (m_state == ComponentState::Running) ? C_LED_GREEN_DARK : C_WIN_DARK;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 最外圈
    QGraphicsEllipseItem *outer = new QGraphicsEllipseItem(-50, -50, 100, 100, this);
    outer->setBrush(QBrush(C_CHAMBER_OUTER));
    outer->setPen(QPen(Qt::darkGray, 1));
    addToGroup(outer);

    // 中圈
    QGraphicsEllipseItem *middle = new QGraphicsEllipseItem(-40, -40, 80, 80, this);
    middle->setBrush(QBrush(C_CHAMBER_INNER));
    middle->setPen(Qt::NoPen);
    addToGroup(middle);

    // 内圈
    QGraphicsEllipseItem *inner = new QGraphicsEllipseItem(-30, -30, 60, 60, this);
    inner->setBrush(QBrush(activeColor));
    inner->setPen(Qt::NoPen);
    addToGroup(inner);

    // 中心点
    QGraphicsEllipseItem *center = new QGraphicsEllipseItem(-8, -8, 16, 16, this);
    center->setBrush(QBrush(C_CHAMBER_CENTER));
    center->setPen(Qt::NoPen);
    addToGroup(center);

    // 十字准星 - 水平线
    QGraphicsLineItem *hLine = new QGraphicsLineItem(-25, 0, 25, 0, this);
    hLine->setPen(QPen(C_CHAMBER_INNER, 2));
    addToGroup(hLine);

    // 十字准星 - 垂直线
    QGraphicsLineItem *vLine = new QGraphicsLineItem(0, -25, 0, 25, this);
    vLine->setPen(QPen(C_CHAMBER_INNER, 2));
    addToGroup(vLine);
}

// ==================== ValveItem ====================
// 阀门：领结形（两个对顶三角形）+ 矩形执行器

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
    QColor fillColor = m_open ? C_LED_GREEN_DARK : C_LED_RED_DARK;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 上三角形
    QPainterPath path;
    path.moveTo(0, -15);
    path.lineTo(12, 0);
    path.lineTo(-12, 0);
    path.closeSubpath();

    QGraphicsPathItem *topTri = new QGraphicsPathItem(path, this);
    topTri->setBrush(QBrush(fillColor));
    topTri->setPen(QPen(Qt::darkGray, 1));
    addToGroup(topTri);

    // 下三角形
    QPainterPath path2;
    path2.moveTo(0, 15);
    path2.lineTo(12, 0);
    path2.lineTo(-12, 0);
    path2.closeSubpath();

    QGraphicsPathItem *bottomTri = new QGraphicsPathItem(path2, this);
    bottomTri->setBrush(QBrush(fillColor));
    bottomTri->setPen(QPen(Qt::darkGray, 1));
    addToGroup(bottomTri);

    // 执行器（矩形）
    QGraphicsRectItem *actuator = new QGraphicsRectItem(-6, -20, 12, 6, this);
    actuator->setBrush(QBrush(C_WIN_DARK));
    actuator->setPen(QPen(Qt::darkGray, 1));
    addToGroup(actuator);
}

// ==================== PumpItem ====================
// 泵：复杂几何体，矩形基座+圆柱形顶部

PumpItem::PumpItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Pump, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void PumpItem::updateAppearance()
{
    QColor activeColor = (m_state == ComponentState::Running) ? C_LED_GREEN_DARK : C_WIN_DARK;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 泵体（矩形）
    QGraphicsRectItem *body = new QGraphicsRectItem(-25, -15, 50, 35, this);
    body->setBrush(QBrush(activeColor));
    body->setPen(QPen(Qt::darkGray, 1));
    addToGroup(body);

    // 泵顶（较小矩形）
    QGraphicsRectItem *top = new QGraphicsRectItem(-18, -25, 36, 12, this);
    top->setBrush(QBrush(C_WIN_MEDIUM));
    top->setPen(QPen(Qt::darkGray, 1));
    addToGroup(top);

    // 泵底座（梯形效果）
    QGraphicsRectItem *base = new QGraphicsRectItem(-28, 18, 56, 8, this);
    base->setBrush(QBrush(C_WIN_DARK));
    base->setPen(QPen(Qt::darkGray, 1));
    addToGroup(base);

    // P 符号
    QGraphicsTextItem *pLabel = new QGraphicsTextItem("P", this);
    pLabel->setDefaultTextColor(Qt::white);
    pLabel->setFont(QFont("Arial", 12, QFont::Bold));
    pLabel->setPos(-5, -8);
    addToGroup(pLabel);
}

// ==================== RFGeneratorItem ====================

RFGeneratorItem::RFGeneratorItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::RFGenerator, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void RFGeneratorItem::updateAppearance()
{
    QColor fillColor = (m_state == ComponentState::Running) ? C_LED_GREEN_DARK : C_WIN_DARK;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 矩形主体
    QGraphicsRectItem *rf = new QGraphicsRectItem(-30, -20, 60, 40, this);
    rf->setBrush(QBrush(fillColor));
    rf->setPen(QPen(Qt::darkGray, 1));
    addToGroup(rf);

    // 频率文字
    QGraphicsTextItem *freq = new QGraphicsTextItem("13.56", this);
    freq->setDefaultTextColor(Qt::white);
    freq->setFont(QFont("Arial", 8));
    freq->setPos(-20, -5);
    addToGroup(freq);

    QGraphicsTextItem *mhz = new QGraphicsTextItem("MHz", this);
    mhz->setDefaultTextColor(Qt::white);
    mhz->setFont(QFont("Arial", 6));
    mhz->setPos(-12, 8);
    addToGroup(mhz);
}

// ==================== ICPGeneratorItem ====================

ICPGeneratorItem::ICPGeneratorItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::ICPGenerator, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void ICPGeneratorItem::updateAppearance()
{
    QColor fillColor = (m_state == ComponentState::Running) ? C_LED_GREEN_DARK : C_WIN_DARK;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 矩形主体
    QGraphicsRectItem *icp = new QGraphicsRectItem(-30, -20, 60, 40, this);
    icp->setBrush(QBrush(fillColor));
    icp->setPen(QPen(Qt::darkGray, 1));
    addToGroup(icp);

    // 频率文字
    QGraphicsTextItem *freq = new QGraphicsTextItem("2", this);
    freq->setDefaultTextColor(Qt::white);
    freq->setFont(QFont("Arial", 14, QFont::Bold));
    freq->setPos(-5, -10);
    addToGroup(freq);

    QGraphicsTextItem *mhz = new QGraphicsTextItem("MHz", this);
    mhz->setDefaultTextColor(Qt::white);
    mhz->setFont(QFont("Arial", 6));
    mhz->setPos(-12, 8);
    addToGroup(mhz);
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
    QColor fillColor = (m_state == ComponentState::Running) ? C_LED_GREEN_DARK : C_WIN_DARK;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // MFC 主体
    QGraphicsRectItem *mfc = new QGraphicsRectItem(-22, -18, 44, 36, this);
    mfc->setBrush(QBrush(fillColor));
    mfc->setPen(QPen(Qt::darkGray, 1));
    addToGroup(mfc);

    // 通道号
    QString chStr = QString("CH%1").arg(m_channel);
    QGraphicsTextItem *chLabel = new QGraphicsTextItem(chStr, this);
    chLabel->setDefaultTextColor(Qt::white);
    chLabel->setFont(QFont("Arial", 9, QFont::Bold));
    chLabel->setPos(-14, -5);
    addToGroup(chLabel);
}

// ==================== PressureGaugeItem ====================

PressureGaugeItem::PressureGaugeItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::PressureGauge, parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void PressureGaugeItem::updateAppearance()
{
    QColor fillColor = (m_state == ComponentState::Running) ? C_LED_GREEN_DARK : C_WIN_DARK;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // 圆形表盘
    QGraphicsEllipseItem *gauge = new QGraphicsEllipseItem(-22, -22, 44, 44, this);
    gauge->setBrush(QBrush(fillColor));
    gauge->setPen(QPen(Qt::darkGray, 1));
    addToGroup(gauge);

    // P 符号
    QGraphicsTextItem *pLabel = new QGraphicsTextItem("P", this);
    pLabel->setDefaultTextColor(Qt::white);
    pLabel->setFont(QFont("Arial", 14, QFont::Bold));
    pLabel->setPos(-5, -10);
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
    QColor fillColor = (m_state == ComponentState::Running) ? C_LED_GREEN_DARK : C_WIN_DARK;

    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    // Chuck 矩形
    QGraphicsRectItem *chuck = new QGraphicsRectItem(-35, -12, 70, 24, this);
    chuck->setBrush(QBrush(fillColor));
    chuck->setPen(QPen(Qt::darkGray, 1));
    addToGroup(chuck);

    // 温度符号
    QGraphicsTextItem *tLabel = new QGraphicsTextItem("°C", this);
    tLabel->setDefaultTextColor(Qt::white);
    tLabel->setFont(QFont("Arial", 7));
    tLabel->setPos(-10, -3);
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

    QColor pipeColor = m_flowing ? C_PIPE_ACTIVE : C_PIPE_INACTIVE;

    QGraphicsLineItem *pipe = new QGraphicsLineItem(-30, 0, 30, 0, this);
    pipe->setPen(QPen(pipeColor, 5));
    addToGroup(pipe);
}

// ==================== BellowsItem ====================
// 波纹管：垂直线条表示

class BellowsItem : public QGraphicsItemGroup
{
public:
    BellowsItem(QGraphicsItem *parent = nullptr) : QGraphicsItemGroup(parent)
    {
        // 绘制波纹效果
        for (int i = 0; i < 6; i++) {
            QGraphicsLineItem *line = new QGraphicsLineItem(-2, i * 4, 2, i * 4, this);
            line->setPen(QPen(C_BELLOW, 2));
            addToGroup(line);
        }
    }
};

// ==================== LEDIndicator ====================

class LEDIndicator : public QGraphicsItemGroup
{
public:
    LEDIndicator(bool on, const QColor &onColor, const QColor &offColor, QGraphicsItem *parent = nullptr)
        : QGraphicsItemGroup(parent), m_on(on), m_onColor(onColor), m_offColor(offColor)
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

        // LED 方块
        QGraphicsRectItem *led = new QGraphicsRectItem(-5, -5, 10, 10, this);
        led->setBrush(QBrush(color));
        led->setPen(QPen(Qt::darkGray, 1));
        addToGroup(led);
    }

    bool m_on;
    QColor m_onColor;
    QColor m_offColor;
};

// ==================== Button3D ====================
// 3D 效果按钮

class Button3D : public QGraphicsItemGroup
{
public:
    Button3D(const QString &text, const QColor &bgColor, QGraphicsItem *parent = nullptr)
        : QGraphicsItemGroup(parent), m_text(text), m_bgColor(bgColor)
    {
        updateAppearance();
    }

private:
    void updateAppearance() {
        for (auto child : childItems()) {
            removeFromGroup(child);
            delete child;
        }

        // 按钮主体（凹陷效果）
        QGraphicsRectItem *btn = new QGraphicsRectItem(0, 0, 80, 24, this);
        btn->setBrush(QBrush(m_bgColor));
        btn->setPen(QPen(Qt::darkGray, 1));
        addToGroup(btn);

        // 3D 效果 - 顶部高光
        QGraphicsLineItem *topLine = new QGraphicsLineItem(1, 1, 78, 1, this);
        topLine->setPen(QPen(Qt::white, 1));
        addToGroup(topLine);

        // 3D 效果 - 左侧高光
        QGraphicsLineItem *leftLine = new QGraphicsLineItem(1, 1, 1, 22, this);
        leftLine->setPen(QPen(Qt::white, 1));
        addToGroup(leftLine);

        // 3D 效果 - 底部阴影
        QGraphicsLineItem *bottomLine = new QGraphicsLineItem(1, 23, 78, 23, this);
        bottomLine->setPen(QPen(Qt::darkGray, 1));
        addToGroup(bottomLine);

        // 3D 效果 - 右侧阴影
        QGraphicsLineItem *rightLine = new QGraphicsLineItem(79, 1, 79, 23, this);
        rightLine->setPen(QPen(Qt::darkGray, 1));
        addToGroup(rightLine);

        // 文字
        QGraphicsTextItem *label = new QGraphicsTextItem(m_text, this);
        label->setDefaultTextColor(Qt::black);
        label->setFont(QFont("Arial", 9));
        label->setPos(10, 5);
        addToGroup(label);
    }

    QString m_text;
    QColor m_bgColor;
};

// ==================== HardwareDiagram ====================

HardwareDiagram::HardwareDiagram(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    // Windows Classic 风格背景
    setBackgroundBrush(QBrush(C_WIN_GRAY));

    m_scene->setSceneRect(0, 0, 1000, 720);
    setupLayout();
}

HardwareDiagram::~HardwareDiagram()
{
}

void HardwareDiagram::setupLayout()
{
    m_scene->clear();
    m_components.clear();

    // ==================== 顶部菜单栏 ====================
    // 菜单栏背景
    QGraphicsRectItem *menuBar = new QGraphicsRectItem(0, 0, 1000, 45);
    menuBar->setBrush(QBrush(C_WIN_LIGHT));
    menuBar->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(menuBar);

    // System 按钮
    Button3D *systemBtn = new Button3D("System", C_WIN_LIGHT);
    systemBtn->setPos(5, 10);
    m_scene->addItem(systemBtn);

    // Process 按钮
    Button3D *processBtn = new Button3D("Process", C_WIN_LIGHT);
    processBtn->setPos(90, 10);
    m_scene->addItem(processBtn);

    // Manager 按钮
    Button3D *managerBtn = new Button3D("Manager", C_WIN_LIGHT);
    managerBtn->setPos(175, 10);
    m_scene->addItem(managerBtn);

    // 状态 LED
    LEDIndicator *sysLed = new LEDIndicator(true, C_LED_BLUE, C_WIN_DARK);
    sysLed->setPos(260, 17);
    m_scene->addItem(sysLed);

    // PUMP CONTROL
    QGraphicsTextItem *pumpLabel = new QGraphicsTextItem("PUMP CONTROL");
    pumpLabel->setFont(QFont("Arial", 9));
    pumpLabel->setDefaultTextColor(Qt::black);
    pumpLabel->setPos(280, 18);
    m_scene->addItem(pumpLabel);

    // 右侧 RED ALERT 按钮
    QGraphicsRectItem *alertBar = new QGraphicsRectItem(800, 5, 195, 35);
    alertBar->setBrush(QBrush(C_LED_RED));
    alertBar->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(alertBar);

    QGraphicsTextItem *alertText = new QGraphicsTextItem("RED ALERT");
    alertText->setFont(QFont("Arial", 12, QFont::Bold));
    alertText->setDefaultTextColor(Qt::white);
    alertText->setPos(855, 15);
    m_scene->addItem(alertText);

    // STOP ALL AUTO PROCESSES 按钮
    QGraphicsRectItem *stopBtn = new QGraphicsRectItem(700, 8, 95, 28);
    stopBtn->setBrush(QBrush(C_LED_GREEN));
    stopBtn->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(stopBtn);

    // 按钮 3D 效果
    QGraphicsLineItem *stopTop = new QGraphicsLineItem(701, 9, 794, 9, m_scene->itemAt(700, 8, QTransform()) ? nullptr : nullptr);
    stopTop->setPen(QPen(Qt::white, 1));
    m_scene->addItem(stopTop);

    QGraphicsTextItem *stopText = new QGraphicsTextItem("STOP ALL");
    stopText->setFont(QFont("Arial", 9, QFont::Bold));
    stopText->setDefaultTextColor(Qt::white);
    stopText->setPos(712, 18);
    m_scene->addItem(stopText);

    // ==================== 状态栏 ====================
    QGraphicsRectItem *statusBar = new QGraphicsRectItem(0, 45, 1000, 25);
    statusBar->setBrush(QBrush(C_WIN_LIGHT));
    statusBar->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(statusBar);

    QGraphicsTextItem *robotStatus = new QGraphicsTextItem("Robot arm standing by");
    robotStatus->setFont(QFont("Arial", 9));
    robotStatus->setDefaultTextColor(Qt::black);
    robotStatus->setPos(10, 52);
    m_scene->addItem(robotStatus);

    // ==================== 主工作区背景 ====================
    QGraphicsRectItem *workArea = new QGraphicsRectItem(0, 70, 1000, 450);
    workArea->setBrush(QBrush(C_WIN_GRAY));
    workArea->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(workArea);

    // ==================== 硬件组件布局 ====================

    // 中心腔室
    ChamberItem *chamber = new ChamberItem("Chamber");
    chamber->setPos(550, 280);
    chamber->setState(ComponentState::Normal);
    m_scene->addItem(chamber);
    m_components["Chamber"] = chamber;

    // RF 发生器 (左侧)
    RFGeneratorItem *rf = new RFGeneratorItem("RF");
    rf->setPos(280, 280);
    rf->setState(ComponentState::Normal);
    m_scene->addItem(rf);
    m_components["RF"] = rf;

    // ICP 发生器 (上方)
    ICPGeneratorItem *icp = new ICPGeneratorItem("ICP");
    icp->setPos(550, 100);
    icp->setState(ComponentState::Normal);
    m_scene->addItem(icp);
    m_components["ICP"] = icp;

    // 压力计 (右侧)
    PressureGaugeItem *pressure = new PressureGaugeItem("Pressure");
    pressure->setPos(800, 280);
    pressure->setState(ComponentState::Normal);
    m_scene->addItem(pressure);
    m_components["Pressure"] = pressure;

    // 机械泵 (左下)
    PumpItem *mp = new PumpItem("MP");
    mp->setPos(180, 450);
    mp->setState(ComponentState::Normal);
    m_scene->addItem(mp);
    m_components["MP"] = mp;

    // 分子泵 (右下)
    PumpItem *tp = new PumpItem("TP");
    tp->setPos(820, 450);
    tp->setState(ComponentState::Normal);
    m_scene->addItem(tp);
    m_components["TP"] = tp;

    // Chuck (下方)
    ChuckItem *chuck = new ChuckItem("Chuck");
    chuck->setPos(550, 480);
    chuck->setState(ComponentState::Normal);
    m_scene->addItem(chuck);
    m_components["Chuck"] = chuck;

    // 气体 MFC (左侧)
    GasMFCItem *mfc1 = new GasMFCItem("MFC1", 1);
    mfc1->setPos(80, 140);
    mfc1->setState(ComponentState::Normal);
    m_scene->addItem(mfc1);
    m_components["MFC1"] = mfc1;

    GasMFCItem *mfc2 = new GasMFCItem("MFC2", 2);
    mfc2->setPos(80, 200);
    mfc2->setState(ComponentState::Normal);
    m_scene->addItem(mfc2);
    m_components["MFC2"] = mfc2;

    GasMFCItem *mfc3 = new GasMFCItem("MFC3", 3);
    mfc3->setPos(80, 260);
    mfc3->setState(ComponentState::Normal);
    m_scene->addItem(mfc3);
    m_components["MFC3"] = mfc3;

    GasMFCItem *mfc4 = new GasMFCItem("MFC4", 4);
    mfc4->setPos(80, 320);
    mfc4->setState(ComponentState::Normal);
    m_scene->addItem(mfc4);
    m_components["MFC4"] = mfc4;

    // 阀门 (领结形)
    ValveItem *v1 = new ValveItem("V1");
    v1->setPos(180, 280);
    v1->setOpen(false);
    m_scene->addItem(v1);
    m_components["V1"] = v1;

    ValveItem *v2 = new ValveItem("V2");
    v2->setPos(380, 280);
    v2->setOpen(false);
    m_scene->addItem(v2);
    m_components["V2"] = v2;

    ValveItem *v3 = new ValveItem("V3");
    v3->setPos(700, 280);
    v3->setOpen(false);
    m_scene->addItem(v3);
    m_components["V3"] = v3;

    ValveItem *v4 = new ValveItem("V4");
    v4->setPos(820, 350);
    v4->setOpen(false);
    m_scene->addItem(v4);
    m_components["V4"] = v4;

    ValveItem *v5 = new ValveItem("V5");
    v5->setPos(380, 450);
    v5->setOpen(false);
    m_scene->addItem(v5);
    m_components["V5"] = v5;

    ValveItem *v6 = new ValveItem("V6");
    v6->setPos(620, 450);
    v6->setOpen(false);
    m_scene->addItem(v6);
    m_components["V6"] = v6;

    // ==================== 管道连接 ====================

    // RF -> Chamber (虚线)
    m_scene->addLine(310, 280, 500, 280, QPen(C_PIPE_INACTIVE, 4));

    // ICP -> Chamber (虚线)
    m_scene->addLine(550, 130, 550, 230, QPen(C_PIPE_INACTIVE, 4));

    // MFC1 -> V1
    m_scene->addLine(102, 140, 180, 140, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(180, 140, 180, 265, QPen(C_PIPE_INACTIVE, 4));

    // MFC2 -> V1
    m_scene->addLine(102, 200, 180, 200, QPen(C_PIPE_INACTIVE, 4));

    // MFC3 -> V2
    m_scene->addLine(102, 260, 150, 260, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(150, 260, 150, 280, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(150, 280, 380, 280, QPen(C_PIPE_INACTIVE, 4));

    // MFC4 -> V2
    m_scene->addLine(102, 320, 150, 320, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(150, 320, 150, 295, QPen(C_PIPE_INACTIVE, 4));

    // Chamber -> V3 -> Pressure
    m_scene->addLine(600, 280, 700, 280, QPen(C_PIPE_INACTIVE, 4));

    // Chamber -> V4 (向下)
    m_scene->addLine(600, 300, 820, 300, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(820, 300, 820, 335, QPen(C_PIPE_INACTIVE, 4));

    // V5 -> MP
    m_scene->addLine(380, 320, 380, 430, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(380, 430, 205, 430, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(205, 430, 205, 435, QPen(C_PIPE_INACTIVE, 4));

    // V6 -> TP
    m_scene->addLine(620, 320, 620, 430, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(620, 430, 795, 430, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(795, 430, 795, 435, QPen(C_PIPE_INACTIVE, 4));

    // Pressure -> V4 -> TP
    m_scene->addLine(822, 280, 870, 280, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(870, 280, 870, 320, QPen(C_PIPE_INACTIVE, 4));
    m_scene->addLine(870, 320, 820, 320, QPen(C_PIPE_INACTIVE, 4));

    // Chuck -> Chamber
    m_scene->addLine(550, 380, 550, 468, QPen(C_PIPE_INACTIVE, 4));

    // ==================== 底部状态面板 ====================
    QGraphicsRectItem *bottomPanel = new QGraphicsRectItem(0, 520, 1000, 200);
    bottomPanel->setBrush(QBrush(C_WIN_LIGHT));
    bottomPanel->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(bottomPanel);

    // 面板标题
    QGraphicsTextItem *panelTitle = new QGraphicsTextItem("System Power Up Lid Down");
    panelTitle->setFont(QFont("Arial", 10, QFont::Bold));
    panelTitle->setDefaultTextColor(Qt::black);
    panelTitle->setPos(10, 530);
    m_scene->addItem(panelTitle);

    // 数据表格区域
    QGraphicsRectItem *tableBg = new QGraphicsRectItem(10, 555, 400, 120);
    tableBg->setBrush(QBrush(C_WIN_GRAY));
    tableBg->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(tableBg);

    // 参数行 1
    QGraphicsTextItem *lidLabel = new QGraphicsTextItem("Lid:");
    lidLabel->setFont(QFont("Arial", 9));
    lidLabel->setDefaultTextColor(Qt::black);
    lidLabel->setPos(20, 565);
    m_scene->addItem(lidLabel);

    QGraphicsRectItem *lidValue = new QGraphicsRectItem(60, 562, 80, 20);
    lidValue->setBrush(QBrush(C_WIN_LIGHT));
    lidValue->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(lidValue);

    QGraphicsTextItem *lidValueText = new QGraphicsTextItem("OPEN");
    lidValueText->setFont(QFont("Arial", 9));
    lidValueText->setDefaultTextColor(Qt::black);
    lidValueText->setPos(70, 565);
    m_scene->addItem(lidValueText);

    // 参数行 2
    QGraphicsTextItem *procLabel = new QGraphicsTextItem("Process interlock:");
    procLabel->setFont(QFont("Arial", 9));
    procLabel->setDefaultTextColor(Qt::black);
    procLabel->setPos(20, 595);
    m_scene->addItem(procLabel);

    // FAULT 显示（红色背景）
    QGraphicsRectItem *faultBg = new QGraphicsRectItem(130, 592, 70, 20);
    faultBg->setBrush(QBrush(C_LED_RED));
    faultBg->setPen(Qt::NoPen);
    m_scene->addItem(faultBg);

    QGraphicsTextItem *faultText = new QGraphicsTextItem("FAULT");
    faultText->setFont(QFont("Arial", 9, QFont::Bold));
    faultText->setDefaultTextColor(Qt::white);
    faultText->setPos(138, 595);
    m_scene->addItem(faultText);

    // 参数行 3 - Pressure
    QGraphicsTextItem *presLabel = new QGraphicsTextItem("Pressure:");
    presLabel->setFont(QFont("Arial", 9));
    presLabel->setDefaultTextColor(Qt::black);
    presLabel->setPos(20, 625);
    m_scene->addItem(presLabel);

    QGraphicsRectItem *presValue = new QGraphicsRectItem(80, 622, 100, 20);
    presValue->setBrush(QBrush(C_WIN_LIGHT));
    presValue->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(presValue);

    QGraphicsTextItem *presValueText = new QGraphicsTextItem("0.000 Torr");
    presValueText->setFont(QFont("Courier New", 9));
    presValueText->setDefaultTextColor(Qt::black);
    presValueText->setPos(90, 625);
    m_scene->addItem(presValueText);

    // 参数行 4 - Vent Time
    QGraphicsTextItem *ventLabel = new QGraphicsTextItem("Vent Time Left:");
    ventLabel->setFont(QFont("Arial", 9));
    ventLabel->setDefaultTextColor(Qt::black);
    ventLabel->setPos(20, 655);
    m_scene->addItem(ventLabel);

    QGraphicsRectItem *ventValue = new QGraphicsRectItem(120, 652, 60, 20);
    ventValue->setBrush(QBrush(C_WIN_LIGHT));
    ventValue->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(ventValue);

    // ==================== 右侧控制按钮 ====================

    // EVACUATE 按钮
    QGraphicsRectItem *evacBtn = new QGraphicsRectItem(450, 560, 90, 35);
    evacBtn->setBrush(QBrush(C_WIN_MEDIUM));
    evacBtn->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(evacBtn);

    // 3D 效果
    QGraphicsLineItem *evacTop = new QGraphicsLineItem(451, 561, 538, 561, m_scene->itemAt(450, 560, QTransform()) ? nullptr : nullptr);
    evacTop->setPen(QPen(Qt::white, 1));
    m_scene->addItem(evacTop);

    QGraphicsTextItem *evacLabel = new QGraphicsTextItem("EVACUATE");
    evacLabel->setFont(QFont("Arial", 9, QFont::Bold));
    evacLabel->setDefaultTextColor(Qt::black);
    evacLabel->setPos(460, 570);
    m_scene->addItem(evacLabel);

    // STOP 按钮
    QGraphicsRectItem *stop2Btn = new QGraphicsRectItem(450, 605, 90, 35);
    stop2Btn->setBrush(QBrush(C_WIN_MEDIUM));
    stop2Btn->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(stop2Btn);

    QGraphicsTextItem *stop2Label = new QGraphicsTextItem("STOP");
    stop2Label->setFont(QFont("Arial", 9, QFont::Bold));
    stop2Label->setDefaultTextColor(Qt::black);
    stop2Label->setPos(475, 615);
    m_scene->addItem(stop2Label);

    // VENT 按钮
    QGraphicsRectItem *ventBtn = new QGraphicsRectItem(450, 650, 90, 35);
    ventBtn->setBrush(QBrush(C_WIN_MEDIUM));
    ventBtn->setPen(QPen(Qt::darkGray, 1));
    m_scene->addItem(ventBtn);

    QGraphicsTextItem *vent2Label = new QGraphicsTextItem("VENT");
    vent2Label->setFont(QFont("Arial", 9, QFont::Bold));
    vent2Label->setDefaultTextColor(Qt::black);
    vent2Label->setPos(472, 660);
    m_scene->addItem(vent2Label);

    // ==================== 右侧状态 LED 面板 ====================

    QGraphicsTextItem *statusTitle = new QGraphicsTextItem("STATUS");
    statusTitle->setFont(QFont("Arial", 10, QFont::Bold));
    statusTitle->setDefaultTextColor(Qt::black);
    statusTitle->setPos(580, 530);
    m_scene->addItem(statusTitle);

    // Process Status
    QGraphicsTextItem *procStatLabel = new QGraphicsTextItem("Process:");
    procStatLabel->setFont(QFont("Arial", 9));
    procStatLabel->setDefaultTextColor(Qt::black);
    procStatLabel->setPos(580, 560);
    m_scene->addItem(procStatLabel);

    LEDIndicator *procLed = new LEDIndicator(false, C_LED_GREEN, C_LED_GREEN_DARK);
    procLed->setPos(650, 557);
    m_scene->addItem(procLed);

    QGraphicsTextItem *procStatText = new QGraphicsTextItem("READY");
    procStatText->setFont(QFont("Arial", 9, QFont::Bold));
    procStatText->setDefaultTextColor(C_LED_GREEN);
    procStatText->setPos(665, 560);
    m_scene->addItem(procStatText);

    // RF Status
    QGraphicsTextItem *rfStatLabel = new QGraphicsTextItem("RF:");
    rfStatLabel->setFont(QFont("Arial", 9));
    rfStatLabel->setDefaultTextColor(Qt::black);
    rfStatLabel->setPos(580, 590);
    m_scene->addItem(rfStatLabel);

    LEDIndicator *rfLed = new LEDIndicator(false, C_LED_GREEN, C_LED_GREEN_DARK);
    rfLed->setPos(620, 587);
    m_scene->addItem(rfLed);

    QGraphicsTextItem *rfStatText = new QGraphicsTextItem("OFF");
    rfStatText->setFont(QFont("Arial", 9, QFont::Bold));
    rfStatText->setDefaultTextColor(C_LED_GREEN_DARK);
    rfStatText->setPos(635, 590);
    m_scene->addItem(rfStatText);

    // ICP Status
    QGraphicsTextItem *icpStatLabel = new QGraphicsTextItem("ICP:");
    icpStatLabel->setFont(QFont("Arial", 9));
    icpStatLabel->setDefaultTextColor(Qt::black);
    icpStatLabel->setPos(580, 620);
    m_scene->addItem(icpStatLabel);

    LEDIndicator *icpLed = new LEDIndicator(false, C_LED_GREEN, C_LED_GREEN_DARK);
    icpLed->setPos(620, 617);
    m_scene->addItem(icpLed);

    QGraphicsTextItem *icpStatText = new QGraphicsTextItem("OFF");
    icpStatText->setFont(QFont("Arial", 9, QFont::Bold));
    icpStatText->setDefaultTextColor(C_LED_GREEN_DARK);
    icpStatText->setPos(635, 620);
    m_scene->addItem(icpStatText);

    // Water Status
    QGraphicsTextItem *waterLabel = new QGraphicsTextItem("Water:");
    waterLabel->setFont(QFont("Arial", 9));
    waterLabel->setDefaultTextColor(Qt::black);
    waterLabel->setPos(580, 650);
    m_scene->addItem(waterLabel);

    // Water Off 警告（红色）
    QGraphicsRectItem *waterOffBg = new QGraphicsRectItem(640, 647, 80, 20);
    waterOffBg->setBrush(QBrush(C_LED_RED));
    waterOffBg->setPen(Qt::NoPen);
    m_scene->addItem(waterOffBg);

    QGraphicsTextItem *waterOffText = new QGraphicsTextItem("Water Off");
    waterOffText->setFont(QFont("Arial", 8, QFont::Bold));
    waterOffText->setDefaultTextColor(Qt::white);
    waterOffText->setPos(650, 650);
    m_scene->addItem(waterOffText);

    // ==================== 图例 ====================

    QGraphicsTextItem *legendTitle = new QGraphicsTextItem("Legend:");
    legendTitle->setFont(QFont("Arial", 9, QFont::Bold));
    legendTitle->setDefaultTextColor(Qt::black);
    legendTitle->setPos(800, 530);
    m_scene->addItem(legendTitle);

    // Valve Open
    LEDIndicator *valveOpenLed = new LEDIndicator(true, C_LED_GREEN_DARK, C_LED_GREEN_DARK);
    valveOpenLed->setPos(800, 560);
    m_scene->addItem(valveOpenLed);

    QGraphicsTextItem *valveOpenLabel = new QGraphicsTextItem("Valve Open");
    valveOpenLabel->setFont(QFont("Arial", 8));
    valveOpenLabel->setDefaultTextColor(Qt::black);
    valveOpenLabel->setPos(815, 563);
    m_scene->addItem(valveOpenLabel);

    // Valve Closed
    LEDIndicator *valveClosedLed = new LEDIndicator(true, C_LED_RED_DARK, C_LED_RED_DARK);
    valveClosedLed->setPos(800, 585);
    m_scene->addItem(valveClosedLed);

    QGraphicsTextItem *valveClosedLabel = new QGraphicsTextItem("Valve Closed");
    valveClosedLabel->setFont(QFont("Arial", 8));
    valveClosedLabel->setDefaultTextColor(Qt::black);
    valveClosedLabel->setPos(815, 588);
    m_scene->addItem(valveClosedLabel);

    // Pump Running
    LEDIndicator *pumpRunLed = new LEDIndicator(true, C_LED_GREEN_DARK, C_LED_GREEN_DARK);
    pumpRunLed->setPos(800, 615);
    m_scene->addItem(pumpRunLed);

    QGraphicsTextItem *pumpRunLabel = new QGraphicsTextItem("Pump Running");
    pumpRunLabel->setFont(QFont("Arial", 8));
    pumpRunLabel->setDefaultTextColor(Qt::black);
    pumpRunLabel->setPos(815, 618);
    m_scene->addItem(pumpRunLabel);
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
