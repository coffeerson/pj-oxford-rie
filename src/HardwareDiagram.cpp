#include "HardwareDiagram.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QGraphicsSimpleTextItem>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <functional>

// Oxford PC 2000 原厂配色精确值
static const QColor C_WIN_GRAY(192, 192, 192);        // Windows 经典灰
static const QColor C_WIN_DARK(128, 128, 128);         // 深灰 3D阴影
static const QColor C_WIN_MEDIUM(160, 160, 160);       // 中灰
static const QColor C_WIN_LIGHT(224, 224, 224);        // 浅灰 3D高光
static const QColor C_TITLE_BLUE(0, 0, 128);           // 标题栏深蓝
static const QColor C_LED_GREEN(0, 200, 0);            // LED 亮绿
static const QColor C_LED_GREEN_DARK(0, 100, 0);       // LED 暗绿
static const QColor C_LED_RED(255, 0, 0);              // LED 红
static const QColor C_LED_RED_DARK(180, 0, 0);         // LED 暗红
static const QColor C_PIPE_BLACK(0, 0, 0);             // 管道黑色
static const QColor C_CHAMBER_OUTER(80, 80, 80);       // 腔室外圈深灰
static const QColor C_CHAMBER_MID(120, 120, 120);      // 腔室中圈
static const QColor C_CHAMBER_INNER(180, 180, 180);    // 腔室内圈浅灰
static const QColor C_WHITE(255, 255, 255);            // 白色
static const QColor C_MENU_BG(212, 208, 200);          // 菜单背景色

// ==================== MenuButton 类 - 顶部下拉菜单按钮 ====================
class MenuButton : public QGraphicsItemGroup
{
public:
    MenuButton(const QString &text, QGraphicsItem *parent = nullptr)
        : QGraphicsItemGroup(parent), m_text(text), m_menuVisible(false), m_hovered(false), m_callback(nullptr)
    {
        setHandlesChildEvents(true);
        setAcceptHoverEvents(true);
        setFlag(QGraphicsItem::ItemIsSelectable);

        // 按钮背景
        m_bg = new QGraphicsRectItem(-50, -12, 100, 24, this);
        m_bg->setBrush(QBrush(C_WIN_GRAY));
        m_bg->setPen(QPen(C_WIN_DARK, 1));
        m_bg->setZValue(0);

        // 3D 效果
        m_topLine = new QGraphicsLineItem(-49, -11, 49, -11, this);
        m_topLine->setPen(QPen(C_WHITE, 1));
        m_topLine->setZValue(1);

        m_leftLine = new QGraphicsLineItem(-49, -11, -49, 11, this);
        m_leftLine->setPen(QPen(C_WHITE, 1));
        m_leftLine->setZValue(1);

        m_bottomLine = new QGraphicsLineItem(-49, 11, 49, 11, this);
        m_bottomLine->setPen(QPen(C_WIN_DARK, 1));
        m_bottomLine->setZValue(1);

        m_rightLine = new QGraphicsLineItem(49, -11, 49, 11, this);
        m_rightLine->setPen(QPen(C_WIN_DARK, 1));
        m_rightLine->setZValue(1);

        // 文字
        m_label = new QGraphicsSimpleTextItem(text, this);
        m_label->setFont(QFont("Arial", 9));
        m_label->setBrush(QBrush(Qt::black));
        m_label->setPos(-m_label->boundingRect().width()/2, -8);
        m_label->setZValue(2);
    }

    void setMenuVisible(bool visible) {
        m_menuVisible = visible;
        updateAppearance();
    }

    bool isMenuVisible() const { return m_menuVisible; }

    void setCallback(std::function<void()> cb) { m_callback = cb; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if (m_callback) m_callback();
        QGraphicsItemGroup::mousePressEvent(event);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override {
        m_hovered = true;
        updateAppearance();
        QGraphicsItemGroup::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override {
        m_hovered = false;
        updateAppearance();
        QGraphicsItemGroup::hoverLeaveEvent(event);
    }

private:
    void updateAppearance() {
        if (m_menuVisible) {
            m_bg->setBrush(QBrush(C_WIN_LIGHT));
        } else if (m_hovered) {
            m_bg->setBrush(QBrush(C_WIN_LIGHT));
        } else {
            m_bg->setBrush(QBrush(C_WIN_GRAY));
        }
    }

    QString m_text;
    bool m_menuVisible;
    bool m_hovered;
    std::function<void()> m_callback;
    QGraphicsRectItem *m_bg;
    QGraphicsLineItem *m_topLine, *m_leftLine, *m_bottomLine, *m_rightLine;
    QGraphicsSimpleTextItem *m_label;
};

// ==================== MenuItem 类 ====================
class MenuItem : public QGraphicsItemGroup
{
public:
    MenuItem(const QString &text, int id, QGraphicsItem *parent = nullptr)
        : QGraphicsItemGroup(parent), m_text(text), m_id(id), m_hovered(false), m_callback(nullptr)
    {
        setHandlesChildEvents(true);
        setAcceptHoverEvents(true);

        m_bg = new QGraphicsRectItem(-45, -10, 175, 24, this);
        m_bg->setBrush(QBrush(C_WIN_LIGHT));
        m_bg->setPen(Qt::NoPen);
        m_bg->setZValue(0);

        m_label = new QGraphicsSimpleTextItem(text, this);
        m_label->setFont(QFont("Arial", 9));
        m_label->setBrush(QBrush(Qt::black));
        m_label->setPos(-40, -6);
        m_label->setZValue(1);
    }

    void setCallback(std::function<void(int)> cb) { m_callback = cb; }

    int id() const { return m_id; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if (m_callback) m_callback(m_id);
        QGraphicsItemGroup::mousePressEvent(event);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override {
        m_hovered = true;
        m_bg->setBrush(QBrush(C_TITLE_BLUE));
        m_label->setBrush(QBrush(Qt::white));
        QGraphicsItemGroup::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override {
        m_hovered = false;
        m_bg->setBrush(QBrush(C_WIN_LIGHT));
        m_label->setBrush(QBrush(Qt::black));
        QGraphicsItemGroup::hoverLeaveEvent(event);
    }

private:
    QString m_text;
    int m_id;
    bool m_hovered;
    std::function<void(int)> m_callback;
    QGraphicsRectItem *m_bg;
    QGraphicsSimpleTextItem *m_label;
};

// ==================== MenuPanel 类 - 下拉菜单面板 ====================
class MenuPanel : public QGraphicsItemGroup
{
public:
    MenuPanel(QGraphicsItem *parent = nullptr)
        : QGraphicsItemGroup(parent), m_visible(false)
    {
        setHandlesChildEvents(true);
        setFlag(QGraphicsItem::ItemIsSelectable, false);
        setFlag(QGraphicsItem::ItemIsMovable, false);
        setZValue(100);

        // 面板背景
        m_panelBg = new QGraphicsRectItem(-50, 12, 180, 300, this);
        m_panelBg->setBrush(QBrush(C_WIN_LIGHT));
        m_panelBg->setPen(QPen(C_WIN_DARK, 2));
        m_panelBg->setZValue(0);

        setVisible(false);
    }

    void setVisible(bool visible) {
        m_visible = visible;
        QGraphicsItemGroup::setVisible(visible);
    }

    bool isVisible() const { return m_visible; }

    void addMenuItem(const QString &text, int id, std::function<void(int)> callback) {
        MenuItem *item = new MenuItem(text, id, this);
        item->setPos(-45, 20 + m_items.size() * 28);
        item->setCallback(callback);
        m_items.append(item);
        updatePanelSize();
    }

    int itemCount() const { return m_items.size(); }

private:
    void updatePanelSize() {
        int height = 20 + m_items.size() * 28 + 10;
        m_panelBg->setRect(-50, 12, 180, height);
    }

    bool m_visible;
    QGraphicsRectItem *m_panelBg;
    QList<MenuItem*> m_items;
};

// ==================== InteractiveButton 类 ====================
class InteractiveButton : public QGraphicsItemGroup
{
public:
    InteractiveButton(const QString &text, QGraphicsItem *parent = nullptr)
        : QGraphicsItemGroup(parent), m_text(text), m_pressed(false), m_hovered(false), m_callback(nullptr)
    {
        setHandlesChildEvents(true);
        setAcceptHoverEvents(true);
        setFlag(QGraphicsItem::ItemIsSelectable);

        m_bg = new QGraphicsRectItem(-40, -12, 80, 24, this);
        m_bg->setBrush(QBrush(C_WIN_GRAY));
        m_bg->setPen(QPen(C_WIN_DARK, 1));
        m_bg->setZValue(0);

        m_topLine = new QGraphicsLineItem(-39, -11, 39, -11, this);
        m_topLine->setPen(QPen(C_WHITE, 1));
        m_topLine->setZValue(1);

        m_leftLine = new QGraphicsLineItem(-39, -11, -39, 11, this);
        m_leftLine->setPen(QPen(C_WHITE, 1));
        m_leftLine->setZValue(1);

        m_bottomLine = new QGraphicsLineItem(-39, 11, 39, 11, this);
        m_bottomLine->setPen(QPen(C_WIN_DARK, 1));
        m_bottomLine->setZValue(1);

        m_rightLine = new QGraphicsLineItem(39, -11, 39, 11, this);
        m_rightLine->setPen(QPen(C_WIN_DARK, 1));
        m_rightLine->setZValue(1);

        m_label = new QGraphicsSimpleTextItem(text, this);
        m_label->setFont(QFont("Arial", 9));
        m_label->setBrush(QBrush(Qt::black));
        m_label->setPos(-m_label->boundingRect().width()/2, -6);
        m_label->setZValue(2);
    }

    QString text() const { return m_text; }
    void setCallback(std::function<void()> cb) { m_callback = cb; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        m_pressed = true;
        updateAppearance();
        QGraphicsItemGroup::mousePressEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        m_pressed = false;
        updateAppearance();
        if (m_callback) m_callback();
        QGraphicsItemGroup::mouseReleaseEvent(event);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override {
        m_hovered = true;
        updateAppearance();
        QGraphicsItemGroup::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override {
        m_hovered = false;
        m_pressed = false;
        updateAppearance();
        QGraphicsItemGroup::hoverLeaveEvent(event);
    }

private:
    void updateAppearance() {
        if (m_pressed) {
            m_bg->setBrush(QBrush(C_WIN_MEDIUM));
            m_topLine->setPen(QPen(C_WIN_DARK, 1));
            m_leftLine->setPen(QPen(C_WIN_DARK, 1));
            m_bottomLine->setPen(QPen(C_WHITE, 1));
            m_rightLine->setPen(QPen(C_WHITE, 1));
        } else {
            m_bg->setBrush(QBrush(C_WIN_GRAY));
            m_topLine->setPen(QPen(C_WHITE, 1));
            m_leftLine->setPen(QPen(C_WHITE, 1));
            m_bottomLine->setPen(QPen(C_WIN_DARK, 1));
            m_rightLine->setPen(QPen(C_WIN_DARK, 1));
        }
    }

    QString m_text;
    bool m_pressed;
    bool m_hovered;
    std::function<void()> m_callback;
    QGraphicsRectItem *m_bg;
    QGraphicsLineItem *m_topLine, *m_leftLine, *m_bottomLine, *m_rightLine;
    QGraphicsSimpleTextItem *m_label;
};

// ==================== LEDIndicator 类 ====================
class LEDIndicator : public QGraphicsItemGroup
{
public:
    LEDIndicator(bool on, QColor onColor, QColor offColor, QGraphicsItem *parent = nullptr)
        : QGraphicsItemGroup(parent), m_on(on), m_onColor(onColor), m_offColor(offColor)
    {
        m_led = new QGraphicsEllipseItem(-6, -6, 12, 12, this);
        m_led->setPen(QPen(Qt::darkGray, 1));
        updateColor();
    }

    void setOn(bool on) {
        m_on = on;
        updateColor();
    }

    bool isOn() const { return m_on; }

private:
    void updateColor() {
        m_led->setBrush(QBrush(m_on ? m_onColor : m_offColor));
    }

    bool m_on;
    QColor m_onColor, m_offColor;
    QGraphicsEllipseItem *m_led;
};

// ==================== HardwareComponent 基类 ====================
HardwareComponent::HardwareComponent(const QString &name, ComponentType type, QGraphicsItem *parent)
    : QGraphicsItemGroup(parent)
    , m_name(name)
    , m_type(type)
    , m_state(ComponentState::Unknown)
    , m_value("")
    , m_nameText(nullptr)
    , m_valueText(nullptr)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);

    m_nameText = new QGraphicsTextItem(this);
    m_nameText->setPlainText(name);
    m_nameText->setDefaultTextColor(Qt::black);
    m_nameText->setFont(QFont("Arial", 7));
    m_nameText->setPos(0, -18);

    m_valueText = new QGraphicsTextItem(this);
    m_valueText->setPlainText("");
    m_valueText->setDefaultTextColor(Qt::black);
    m_valueText->setFont(QFont("Arial", 7));
    m_valueText->setPos(0, 10);
}

void HardwareComponent::setState(ComponentState state)
{
    m_state = state;
    updateAppearance();
}

void HardwareComponent::setValue(const QString &value)
{
    m_value = value;
    if (m_valueText) {
        m_valueText->setPlainText(value);
    }
}

QRectF HardwareComponent::boundingRect() const
{
    return QRectF(0, -20, 60, 55);
}

// ==================== ChamberItem ====================
ChamberItem::ChamberItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Chamber, parent)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setToolTip("Process Chamber - Right-click for options");
}

void ChamberItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    bool running = (m_state == ComponentState::Running);

    // 最外圈
    QGraphicsEllipseItem *outer = new QGraphicsEllipseItem(-55, -55, 110, 110, this);
    outer->setBrush(QBrush(C_CHAMBER_OUTER));
    outer->setPen(QPen(Qt::darkGray, 2));
    addToGroup(outer);

    // 第二圈
    QGraphicsEllipseItem *ring2 = new QGraphicsEllipseItem(-45, -45, 90, 90, this);
    ring2->setBrush(QBrush(C_CHAMBER_MID));
    ring2->setPen(Qt::NoPen);
    addToGroup(ring2);

    // 第三圈
    QGraphicsEllipseItem *ring3 = new QGraphicsEllipseItem(-35, -35, 70, 70, this);
    ring3->setBrush(QBrush(C_CHAMBER_INNER));
    ring3->setPen(Qt::NoPen);
    addToGroup(ring3);

    // 内圈
    QColor innerColor = running ? C_LED_GREEN_DARK : QColor(60, 60, 60);
    QGraphicsEllipseItem *inner = new QGraphicsEllipseItem(-25, -25, 50, 50, this);
    inner->setBrush(QBrush(innerColor));
    inner->setPen(Qt::NoPen);
    addToGroup(inner);

    // 十字准星
    QPen crossPen(running ? C_LED_GREEN : QColor(100, 100, 100), 2);
    QGraphicsLineItem *hLine = new QGraphicsLineItem(-20, 0, 20, 0, this);
    hLine->setPen(crossPen);
    addToGroup(hLine);

    QGraphicsLineItem *vLine = new QGraphicsLineItem(0, -20, 0, 20, this);
    vLine->setPen(crossPen);
    addToGroup(vLine);

    // 中心点
    QGraphicsEllipseItem *center = new QGraphicsEllipseItem(-5, -5, 10, 10, this);
    center->setBrush(QBrush(running ? C_LED_GREEN : QColor(80, 80, 80)));
    center->setPen(Qt::NoPen);
    addToGroup(center);

    // 标签
    QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem("1", this);
    label->setFont(QFont("Arial", 10, QFont::Bold));
    label->setBrush(QBrush(Qt::white));
    label->setPos(-4, -6);
}

void ChamberItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
    } else {
        qDebug() << "Chamber clicked!";
    }
    QGraphicsItemGroup::mousePressEvent(event);
}

void ChamberItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Chamber double-clicked!";
    QGraphicsItemGroup::mouseDoubleClickEvent(event);
}

// ==================== ValveItem ====================
ValveItem::ValveItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Valve, parent)
    , m_open(false)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setToolTip("Valve - Click to toggle, Right-click for options");
}

void ValveItem::setOpen(bool open)
{
    m_open = open;
    updateAppearance();
}

void ValveItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    QColor valveColor = m_open ? C_LED_GREEN_DARK : C_LED_RED_DARK;
    QColor actuatorColor = m_open ? C_LED_GREEN : C_LED_RED;

    // 执行器
    QGraphicsRectItem *actuator = new QGraphicsRectItem(-8, -25, 16, 8, this);
    actuator->setBrush(QBrush(actuatorColor));
    actuator->setPen(QPen(Qt::darkGray, 1));
    addToGroup(actuator);

    // 连接线
    QGraphicsLineItem *connector = new QGraphicsLineItem(0, -17, 0, -10, this);
    connector->setPen(QPen(Qt::darkGray, 2));
    addToGroup(connector);

    // 上三角形
    QPainterPath topPath;
    topPath.moveTo(0, -10);
    topPath.lineTo(15, 0);
    topPath.lineTo(-15, 0);
    topPath.closeSubpath();

    QGraphicsPathItem *topTri = new QGraphicsPathItem(topPath, this);
    topTri->setBrush(QBrush(valveColor));
    topTri->setPen(QPen(Qt::black, 1.5));
    addToGroup(topTri);

    // 下三角形
    QPainterPath bottomPath;
    bottomPath.moveTo(0, 15);
    bottomPath.lineTo(15, 0);
    bottomPath.lineTo(-15, 0);
    bottomPath.closeSubpath();

    QGraphicsPathItem *bottomTri = new QGraphicsPathItem(bottomPath, this);
    bottomTri->setBrush(QBrush(valveColor));
    bottomTri->setPen(QPen(Qt::black, 1.5));
    addToGroup(bottomTri);

    // 中间轴
    QGraphicsEllipseItem *pivot = new QGraphicsEllipseItem(-3, -3, 6, 6, this);
    pivot->setBrush(QBrush(Qt::darkGray));
    pivot->setPen(Qt::NoPen);
    addToGroup(pivot);

    // 状态标签
    QString state = m_open ? "OPEN" : "CLOSED";
    QGraphicsSimpleTextItem *stateLabel = new QGraphicsSimpleTextItem(state, this);
    stateLabel->setFont(QFont("Arial", 7));
    stateLabel->setBrush(QBrush(m_open ? C_LED_GREEN : C_LED_RED));
    stateLabel->setPos(-15, 18);
}

void ValveItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
    } else {
        setOpen(!m_open);
        qDebug() << "Valve" << m_name << (m_open ? "OPENED" : "CLOSED");
    }
    QGraphicsItemGroup::mousePressEvent(event);
}

// ==================== PumpItem ====================
PumpItem::PumpItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Pump, parent)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setToolTip("Pump - Right-click for options");
}

void PumpItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    bool running = (m_state == ComponentState::Running);
    QColor bodyColor = running ? C_LED_GREEN_DARK : C_WIN_DARK;

    // 泵体
    QGraphicsRectItem *body = new QGraphicsRectItem(-30, -10, 60, 35, this);
    body->setBrush(QBrush(bodyColor));
    body->setPen(QPen(Qt::darkGray, 1));
    addToGroup(body);

    // 泵顶
    QGraphicsRectItem *top = new QGraphicsRectItem(-22, -20, 44, 12, this);
    top->setBrush(QBrush(C_WIN_MEDIUM));
    top->setPen(QPen(Qt::darkGray, 1));
    addToGroup(top);

    // 圆柱顶面
    QGraphicsEllipseItem *topCap = new QGraphicsEllipseItem(-22, -25, 44, 10, this);
    topCap->setBrush(QBrush(C_WIN_LIGHT));
    topCap->setPen(QPen(Qt::darkGray, 1));
    addToGroup(topCap);

    // 泵底座
    QGraphicsRectItem *base = new QGraphicsRectItem(-35, 23, 70, 6, this);
    base->setBrush(QBrush(C_WIN_DARK));
    base->setPen(Qt::NoPen);
    addToGroup(base);

    // P 标记
    QGraphicsSimpleTextItem *pLabel = new QGraphicsSimpleTextItem("P", this);
    pLabel->setFont(QFont("Arial", 14, QFont::Bold));
    pLabel->setBrush(QBrush(Qt::white));
    pLabel->setPos(-6, -5);
    addToGroup(pLabel);

    // 冷却鳍片
    for (int i = 0; i < 4; i++) {
        QGraphicsRectItem *fin = new QGraphicsRectItem(-30 + i*16, 5, 8, 15, this);
        fin->setBrush(QBrush(C_WIN_MEDIUM));
        fin->setPen(Qt::NoPen);
        addToGroup(fin);
    }
}

void PumpItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
    } else {
        qDebug() << "Pump" << m_name << "clicked!";
    }
    QGraphicsItemGroup::mousePressEvent(event);
}

// ==================== RFGeneratorItem ====================
RFGeneratorItem::RFGeneratorItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::RFGenerator, parent)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setToolTip("RF Generator - Right-click for options");
}

void RFGeneratorItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    bool active = (m_state == ComponentState::Running);
    QColor fillColor = active ? C_LED_GREEN_DARK : C_WIN_DARK;

    QGraphicsRectItem *rf = new QGraphicsRectItem(-35, -25, 70, 50, this);
    rf->setBrush(QBrush(fillColor));
    rf->setPen(QPen(Qt::darkGray, 1));
    addToGroup(rf);

    QGraphicsSimpleTextItem *freq = new QGraphicsSimpleTextItem("13.56", this);
    freq->setFont(QFont("Arial", 11, QFont::Bold));
    freq->setBrush(QBrush(Qt::white));
    freq->setPos(-22, -18);
    addToGroup(freq);

    QGraphicsSimpleTextItem *mhz = new QGraphicsSimpleTextItem("MHz", this);
    mhz->setFont(QFont("Arial", 7));
    mhz->setBrush(QBrush(Qt::white));
    mhz->setPos(-15, 0);
    addToGroup(mhz);

    QGraphicsSimpleTextItem *rfLabel = new QGraphicsSimpleTextItem("RF", this);
    rfLabel->setFont(QFont("Arial", 8));
    rfLabel->setBrush(QBrush(QColor(180, 180, 180)));
    rfLabel->setPos(-8, 15);
    addToGroup(rfLabel);
}

void RFGeneratorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
    }
    QGraphicsItemGroup::mousePressEvent(event);
}

// ==================== ICPGeneratorItem ====================
ICPGeneratorItem::ICPGeneratorItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::ICPGenerator, parent)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setToolTip("ICP Source - Right-click for options");
}

void ICPGeneratorItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    bool active = (m_state == ComponentState::Running);
    QColor fillColor = active ? C_LED_GREEN_DARK : C_WIN_DARK;

    QGraphicsRectItem *icp = new QGraphicsRectItem(-40, -30, 80, 60, this);
    icp->setBrush(QBrush(fillColor));
    icp->setPen(QPen(Qt::darkGray, 1));
    addToGroup(icp);

    QGraphicsSimpleTextItem *freq = new QGraphicsSimpleTextItem("2", this);
    freq->setFont(QFont("Arial", 18, QFont::Bold));
    freq->setBrush(QBrush(Qt::white));
    freq->setPos(-8, -22);
    addToGroup(freq);

    QGraphicsSimpleTextItem *mhz = new QGraphicsSimpleTextItem("MHz", this);
    mhz->setFont(QFont("Arial", 8));
    mhz->setBrush(QBrush(Qt::white));
    mhz->setPos(-15, 2);
    addToGroup(mhz);

    QGraphicsSimpleTextItem *icpLabel = new QGraphicsSimpleTextItem("ICP", this);
    icpLabel->setFont(QFont("Arial", 9));
    icpLabel->setBrush(QBrush(QColor(180, 180, 180)));
    icpLabel->setPos(-12, 20);
    addToGroup(icpLabel);
}

void ICPGeneratorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
    }
    QGraphicsItemGroup::mousePressEvent(event);
}

// ==================== GasMFCItem ====================
GasMFCItem::GasMFCItem(const QString &name, int channel, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::GasMFC, parent)
    , m_channel(channel)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setToolTip("Gas MFC - Right-click for flow settings");
}

void GasMFCItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    bool active = (m_state == ComponentState::Running);
    QColor fillColor = active ? C_LED_GREEN_DARK : C_WIN_DARK;

    QGraphicsRectItem *mfc = new QGraphicsRectItem(-25, -20, 50, 40, this);
    mfc->setBrush(QBrush(fillColor));
    mfc->setPen(QPen(Qt::darkGray, 1));
    addToGroup(mfc);

    QString chStr = QString("CH%1").arg(m_channel);
    QGraphicsSimpleTextItem *ch = new QGraphicsSimpleTextItem(chStr, this);
    ch->setFont(QFont("Arial", 10, QFont::Bold));
    ch->setBrush(QBrush(Qt::white));
    ch->setPos(-15, -15);
    addToGroup(ch);

    QGraphicsSimpleTextItem *gas = new QGraphicsSimpleTextItem(m_name, this);
    gas->setFont(QFont("Arial", 8));
    gas->setBrush(QBrush(QColor(180, 180, 180)));
    gas->setPos(-12, 0);
    addToGroup(gas);

    QString valStr = m_value.isEmpty() ? "0.0" : m_value;
    QGraphicsSimpleTextItem *val = new QGraphicsSimpleTextItem(valStr, this);
    val->setFont(QFont("Courier New", 9));
    val->setBrush(QBrush(Qt::white));
    val->setPos(-15, 12);
    addToGroup(val);
}

void GasMFCItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
    }
    QGraphicsItemGroup::mousePressEvent(event);
}

// ==================== PressureGaugeItem ====================
PressureGaugeItem::PressureGaugeItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::PressureGauge, parent)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setToolTip("Pressure Gauge - Right-click for options");
}

void PressureGaugeItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    QGraphicsRectItem *gauge = new QGraphicsRectItem(-30, -25, 60, 50, this);
    gauge->setBrush(QBrush(C_WIN_GRAY));
    gauge->setPen(QPen(Qt::darkGray, 1));
    addToGroup(gauge);

    QGraphicsRectItem *display = new QGraphicsRectItem(-25, -20, 50, 20, this);
    display->setBrush(QBrush(Qt::black));
    display->setPen(Qt::NoPen);
    addToGroup(display);

    QString valStr = m_value.isEmpty() ? "0.00" : m_value;
    QGraphicsSimpleTextItem *val = new QGraphicsSimpleTextItem(valStr, this);
    val->setFont(QFont("Courier New", 10, QFont::Bold));
    val->setBrush(QBrush(C_LED_GREEN));
    val->setPos(-22, -17);
    addToGroup(val);

    QGraphicsSimpleTextItem *unit = new QGraphicsSimpleTextItem("mbar", this);
    unit->setFont(QFont("Arial", 7));
    unit->setBrush(QBrush(QColor(180, 180, 180)));
    unit->setPos(-12, 0);
    addToGroup(unit);

    QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem("Pirani", this);
    label->setFont(QFont("Arial", 8));
    label->setBrush(QBrush(Qt::black));
    label->setPos(-18, 25);
    addToGroup(label);
}

void PressureGaugeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
    }
    QGraphicsItemGroup::mousePressEvent(event);
}

// ==================== ChuckItem ====================
ChuckItem::ChuckItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Chuck, parent)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setToolTip("Chuck - Right-click for temperature settings");
}

void ChuckItem::updateAppearance()
{
    for (auto child : childItems()) {
        if (child != m_nameText && child != m_valueText) {
            removeFromGroup(child);
            delete child;
        }
    }

    bool active = (m_state == ComponentState::Running);
    QColor fillColor = active ? C_LED_GREEN_DARK : C_WIN_DARK;

    QGraphicsRectItem *chuck = new QGraphicsRectItem(-40, -15, 80, 30, this);
    chuck->setBrush(QBrush(fillColor));
    chuck->setPen(QPen(Qt::darkGray, 1));
    addToGroup(chuck);

    QString tempStr = m_value.isEmpty() ? "25" : m_value;
    QGraphicsSimpleTextItem *temp = new QGraphicsSimpleTextItem(tempStr, this);
    temp->setFont(QFont("Courier New", 14, QFont::Bold));
    temp->setBrush(QBrush(Qt::white));
    temp->setPos(-18, -10);
    addToGroup(temp);

    QGraphicsSimpleTextItem *unit = new QGraphicsSimpleTextItem("C", this);
    unit->setFont(QFont("Arial", 10));
    unit->setBrush(QBrush(Qt::white));
    unit->setPos(12, -8);
    addToGroup(unit);

    QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem("Chuck", this);
    label->setFont(QFont("Arial", 8));
    label->setBrush(QBrush(QColor(180, 180, 180)));
    label->setPos(-20, 15);
    addToGroup(label);
}

void ChuckItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
    }
    QGraphicsItemGroup::mousePressEvent(event);
}

// ==================== PipeItem ====================
PipeItem::PipeItem(const QString &name, QGraphicsItem *parent)
    : HardwareComponent(name, ComponentType::Pipe, parent)
    , m_flowing(false)
{
    setHandlesChildEvents(true);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
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

    QColor pipeColor = m_flowing ? C_LED_GREEN : C_PIPE_BLACK;
    QPen pipePen(pipeColor, 4);
}

// ==================== HardwareDiagram 主类 ====================

HardwareDiagram::HardwareDiagram(QWidget *parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setBackgroundBrush(QBrush(C_WIN_GRAY));
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_scene->setSceneRect(0, 0, 1100, 750);
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
    QGraphicsRectItem *menuBar = new QGraphicsRectItem(0, 0, 1100, 45);
    menuBar->setBrush(QBrush(C_WIN_GRAY));
    menuBar->setPen(QPen(C_WIN_DARK, 1));
    menuBar->setZValue(0);
    m_scene->addItem(menuBar);

    // System 按钮
    m_systemBtn = new MenuButton("System");
    m_systemBtn->setPos(5, 12);
    m_systemBtn->setZValue(10);
    m_scene->addItem(m_systemBtn);

    // System 下拉菜单
    m_systemMenu = new MenuPanel();
    m_systemMenu->setPos(5, 24);
    m_systemMenu->addMenuItem("Pumping", 1, nullptr);
    m_systemMenu->addMenuItem("Venting", 2, nullptr);
    m_systemMenu->addMenuItem("Configuration", 3, nullptr);
    m_systemMenu->addMenuItem("Service", 4, nullptr);
    m_systemMenu->addMenuItem("Passwords", 5, nullptr);
    m_systemMenu->addMenuItem("Status...", 6, nullptr);
    m_systemMenu->setVisible(false);
    m_scene->addItem(m_systemMenu);

    // Process 按钮
    m_processBtn = new MenuButton("Process");
    m_processBtn->setPos(110, 12);
    m_processBtn->setZValue(10);
    m_scene->addItem(m_processBtn);

    // Process 下拉菜单
    m_processMenu = new MenuPanel();
    m_processMenu->setPos(110, 24);
    m_processMenu->addMenuItem("Start", 10, nullptr);
    m_processMenu->addMenuItem("Stop", 11, nullptr);
    m_processMenu->addMenuItem("Edit Recipe", 12, nullptr);
    m_processMenu->addMenuItem("New Recipe", 13, nullptr);
    m_processMenu->addMenuItem("Load Recipe", 14, nullptr);
    m_processMenu->addMenuItem("Save Recipe", 15, nullptr);
    m_processMenu->setVisible(false);
    m_scene->addItem(m_processMenu);

    // Manager 按钮
    m_managerBtn = new MenuButton("Manager");
    m_managerBtn->setPos(215, 12);
    m_managerBtn->setZValue(10);
    m_scene->addItem(m_managerBtn);

    // Manager 下拉菜单
    m_managerMenu = new MenuPanel();
    m_managerMenu->setPos(215, 24);
    m_managerMenu->addMenuItem("Log on", 20, nullptr);
    m_managerMenu->addMenuItem("Log off", 21, nullptr);
    m_managerMenu->addMenuItem("Production Mode", 22, nullptr);
    m_managerMenu->addMenuItem("Edit Users", 23, nullptr);
    m_managerMenu->addMenuItem("Preferences", 24, nullptr);
    m_managerMenu->setVisible(false);
    m_scene->addItem(m_managerMenu);

    // 设置菜单按钮回调
    m_systemBtn->setCallback([this]() { onSystemMenuToggle(); });
    m_processBtn->setCallback([this]() { onProcessMenuToggle(); });
    m_managerBtn->setCallback([this]() { onManagerMenuToggle(); });

    // PUMP CONTROL
    QGraphicsSimpleTextItem *pumpLabel = new QGraphicsSimpleTextItem("PUMP CONTROL");
    pumpLabel->setFont(QFont("Arial", 9));
    pumpLabel->setBrush(QBrush(Qt::black));
    pumpLabel->setPos(320, 16);
    pumpLabel->setZValue(10);
    m_scene->addItem(pumpLabel);

    // 右侧 RED ALERT
    QGraphicsRectItem *alertBar = new QGraphicsRectItem(850, 5, 240, 35);
    alertBar->setBrush(QBrush(C_LED_RED));
    alertBar->setPen(QPen(Qt::darkGray, 1));
    alertBar->setZValue(10);
    m_scene->addItem(alertBar);

    QGraphicsSimpleTextItem *alertText = new QGraphicsSimpleTextItem("RED ALERT");
    alertText->setFont(QFont("Arial", 14, QFont::Bold));
    alertText->setBrush(QBrush(Qt::white));
    alertText->setPos(920, 13);
    alertText->setZValue(11);
    m_scene->addItem(alertText);

    // STOP ALL AUTO PROCESSES
    QGraphicsRectItem *stopBtn = new QGraphicsRectItem(700, 8, 140, 30);
    stopBtn->setBrush(QBrush(C_LED_GREEN));
    stopBtn->setPen(QPen(Qt::darkGray, 1));
    stopBtn->setZValue(10);
    m_scene->addItem(stopBtn);

    QGraphicsSimpleTextItem *stopText = new QGraphicsSimpleTextItem("STOP ALL AUTO PROCESSES");
    stopText->setFont(QFont("Arial", 9, QFont::Bold));
    stopText->setBrush(QBrush(Qt::white));
    stopText->setPos(712, 18);
    stopText->setZValue(11);
    m_scene->addItem(stopText);

    // ==================== 状态栏 ====================
    QGraphicsRectItem *statusBar = new QGraphicsRectItem(0, 45, 1100, 28);
    statusBar->setBrush(QBrush(C_WIN_LIGHT));
    statusBar->setPen(QPen(Qt::darkGray, 1));
    statusBar->setZValue(0);
    m_scene->addItem(statusBar);

    QGraphicsSimpleTextItem *robotStatus = new QGraphicsSimpleTextItem("Robot arm standing by");
    robotStatus->setFont(QFont("Arial", 9));
    robotStatus->setBrush(QBrush(Qt::black));
    robotStatus->setPos(10, 54);
    robotStatus->setZValue(10);
    m_scene->addItem(robotStatus);

    // ==================== Mimic 区域 ====================
    QGraphicsRectItem *mimicBg = new QGraphicsRectItem(0, 73, 800, 450);
    mimicBg->setBrush(QBrush(C_MENU_BG));
    mimicBg->setPen(QPen(Qt::darkGray, 1));
    mimicBg->setZValue(0);
    m_scene->addItem(mimicBg);

    // Mimic 区域标题
    QGraphicsSimpleTextItem *mimicTitle = new QGraphicsSimpleTextItem("MIMIC DIAGRAM");
    mimicTitle->setFont(QFont("Arial", 10, QFont::Bold));
    mimicTitle->setBrush(QBrush(Qt::black));
    mimicTitle->setPos(10, 80);
    mimicTitle->setZValue(5);
    m_scene->addItem(mimicTitle);

    // ==================== 绘制管道 ====================
    QPen pipePen(C_PIPE_BLACK, 4);

    // 主管道 - 腔室到泵
    addPipeLine(550, 330, 550, 420, pipePen);
    addPipeLine(550, 420, 200, 420, pipePen);
    addPipeLine(200, 420, 200, 450, pipePen);

    // 腔室到 RF
    addPipeLine(500, 280, 350, 280, pipePen);

    // 腔室到 ICP
    addPipeLine(550, 230, 550, 180, pipePen);

    // 压力计连接
    addPipeLine(600, 280, 750, 280, pipePen);

    // 气体管道
    addPipeLine(100, 250, 100, 350, pipePen);
    addPipeLine(100, 350, 500, 350, pipePen);
    addPipeLine(500, 350, 500, 300, pipePen);

    // 分子泵连接
    addPipeLine(600, 330, 700, 330, pipePen);
    addPipeLine(700, 330, 700, 420, pipePen);
    addPipeLine(700, 420, 850, 420, pipePen);

    // ==================== 组件 ====================

    ChamberItem *chamber = new ChamberItem("Chamber");
    chamber->setPos(550, 280);
    chamber->setState(ComponentState::Normal);
    chamber->setZValue(5);
    m_scene->addItem(chamber);
    m_components["Chamber"] = chamber;

    RFGeneratorItem *rf = new RFGeneratorItem("RF Generator");
    rf->setPos(320, 280);
    rf->setState(ComponentState::Normal);
    rf->setZValue(5);
    m_scene->addItem(rf);
    m_components["RF"] = rf;

    ICPGeneratorItem *icp = new ICPGeneratorItem("ICP Source");
    icp->setPos(550, 150);
    icp->setState(ComponentState::Normal);
    icp->setZValue(5);
    m_scene->addItem(icp);
    m_components["ICP"] = icp;

    PressureGaugeItem *pressure = new PressureGaugeItem("Pressure");
    pressure->setPos(780, 280);
    pressure->setState(ComponentState::Normal);
    pressure->setZValue(5);
    m_scene->addItem(pressure);
    m_components["Pressure"] = pressure;

    PumpItem *mp = new PumpItem("MP");
    mp->setPos(200, 470);
    mp->setState(ComponentState::Normal);
    mp->setZValue(5);
    m_scene->addItem(mp);
    m_components["MP"] = mp;

    PumpItem *tp = new PumpItem("TP");
    tp->setPos(850, 470);
    tp->setState(ComponentState::Normal);
    tp->setZValue(5);
    m_scene->addItem(tp);
    m_components["TP"] = tp;

    ChuckItem *chuck = new ChuckItem("Chuck");
    chuck->setPos(550, 400);
    chuck->setState(ComponentState::Normal);
    chuck->setZValue(5);
    m_scene->addItem(chuck);
    m_components["Chuck"] = chuck;

    // 气体 MFC
    QStringList gases = {"Ar", "O2", "N2", "CF4"};
    for (int i = 0; i < 4; i++) {
        GasMFCItem *mfc = new GasMFCItem(gases[i], i+1);
        mfc->setPos(70, 180 + i * 50);
        mfc->setState(ComponentState::Normal);
        mfc->setZValue(5);
        m_scene->addItem(mfc);
        m_components[gases[i]] = mfc;
    }

    // 阀门
    ValveItem *v1 = new ValveItem("V1");
    v1->setPos(550, 370);
    v1->setOpen(false);
    v1->setZValue(5);
    m_scene->addItem(v1);
    m_components["V1"] = v1;

    ValveItem *v2 = new ValveItem("V2");
    v2->setPos(470, 280);
    v2->setOpen(false);
    v2->setZValue(5);
    m_scene->addItem(v2);
    m_components["V2"] = v2;

    ValveItem *v3 = new ValveItem("V3");
    v3->setPos(630, 280);
    v3->setOpen(false);
    v3->setZValue(5);
    m_scene->addItem(v3);
    m_components["V3"] = v3;

    ValveItem *v4 = new ValveItem("V4");
    v4->setPos(200, 390);
    v4->setOpen(false);
    v4->setZValue(5);
    m_scene->addItem(v4);
    m_components["V4"] = v4;

    ValveItem *v5 = new ValveItem("V5");
    v5->setPos(700, 370);
    v5->setOpen(false);
    v5->setZValue(5);
    m_scene->addItem(v5);
    m_components["V5"] = v5;

    ValveItem *v6 = new ValveItem("V6");
    v6->setPos(100, 300);
    v6->setOpen(false);
    v6->setZValue(5);
    m_scene->addItem(v6);
    m_components["V6"] = v6;

    // ==================== 右侧状态面板 ====================
    QGraphicsRectItem *statusPanel = new QGraphicsRectItem(800, 73, 300, 250);
    statusPanel->setBrush(QBrush(C_WIN_GRAY));
    statusPanel->setPen(QPen(Qt::darkGray, 1));
    statusPanel->setZValue(0);
    m_scene->addItem(statusPanel);

    QGraphicsSimpleTextItem *statusTitle = new QGraphicsSimpleTextItem("STATUS");
    statusTitle->setFont(QFont("Arial", 11, QFont::Bold));
    statusTitle->setBrush(QBrush(Qt::black));
    statusTitle->setPos(815, 85);
    statusTitle->setZValue(10);
    m_scene->addItem(statusTitle);

    // Process
    QGraphicsSimpleTextItem *procLabel = new QGraphicsSimpleTextItem("Process:");
    procLabel->setFont(QFont("Arial", 9));
    procLabel->setBrush(QBrush(Qt::black));
    procLabel->setPos(815, 115);
    procLabel->setZValue(10);
    m_scene->addItem(procLabel);

    LEDIndicator *procLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN_DARK);
    procLed->setPos(890, 110);
    procLed->setZValue(10);
    m_scene->addItem(procLed);

    QGraphicsSimpleTextItem *procStat = new QGraphicsSimpleTextItem("READY");
    procStat->setFont(QFont("Arial", 9, QFont::Bold));
    procStat->setBrush(QBrush(C_LED_GREEN));
    procStat->setPos(905, 115);
    procStat->setZValue(10);
    m_scene->addItem(procStat);

    // RF
    QGraphicsSimpleTextItem *rfLabel = new QGraphicsSimpleTextItem("RF:");
    rfLabel->setFont(QFont("Arial", 9));
    rfLabel->setBrush(QBrush(Qt::black));
    rfLabel->setPos(815, 150);
    rfLabel->setZValue(10);
    m_scene->addItem(rfLabel);

    LEDIndicator *rfLed = new LEDIndicator(false, C_LED_GREEN, C_LED_GREEN_DARK);
    rfLed->setPos(850, 145);
    rfLed->setZValue(10);
    m_scene->addItem(rfLed);

    QGraphicsSimpleTextItem *rfStat = new QGraphicsSimpleTextItem("OFF");
    rfStat->setFont(QFont("Arial", 9, QFont::Bold));
    rfStat->setBrush(QBrush(C_LED_GREEN_DARK));
    rfStat->setPos(865, 150);
    rfStat->setZValue(10);
    m_scene->addItem(rfStat);

    // ICP
    QGraphicsSimpleTextItem *icpLabel = new QGraphicsSimpleTextItem("ICP:");
    icpLabel->setFont(QFont("Arial", 9));
    icpLabel->setBrush(QBrush(Qt::black));
    icpLabel->setPos(815, 185);
    icpLabel->setZValue(10);
    m_scene->addItem(icpLabel);

    LEDIndicator *icpLed = new LEDIndicator(false, C_LED_GREEN, C_LED_GREEN_DARK);
    icpLed->setPos(850, 180);
    icpLed->setZValue(10);
    m_scene->addItem(icpLed);

    QGraphicsSimpleTextItem *icpStat = new QGraphicsSimpleTextItem("OFF");
    icpStat->setFont(QFont("Arial", 9, QFont::Bold));
    icpStat->setBrush(QBrush(C_LED_GREEN_DARK));
    icpStat->setPos(865, 185);
    icpStat->setZValue(10);
    m_scene->addItem(icpStat);

    // Water
    QGraphicsSimpleTextItem *waterLabel = new QGraphicsSimpleTextItem("Water:");
    waterLabel->setFont(QFont("Arial", 9));
    waterLabel->setBrush(QBrush(Qt::black));
    waterLabel->setPos(815, 220);
    waterLabel->setZValue(10);
    m_scene->addItem(waterLabel);

    QGraphicsRectItem *waterOffBg = new QGraphicsRectItem(880, 217, 90, 22);
    waterOffBg->setBrush(QBrush(C_LED_RED));
    waterOffBg->setPen(Qt::NoPen);
    waterOffBg->setZValue(10);
    m_scene->addItem(waterOffBg);

    QGraphicsSimpleTextItem *waterOff = new QGraphicsSimpleTextItem("Water Off");
    waterOff->setFont(QFont("Arial", 9, QFont::Bold));
    waterOff->setBrush(QBrush(Qt::white));
    waterOff->setPos(890, 222);
    waterOff->setZValue(11);
    m_scene->addItem(waterOff);

    // Pressure
    QGraphicsSimpleTextItem *presLabel = new QGraphicsSimpleTextItem("Pressure:");
    presLabel->setFont(QFont("Arial", 9));
    presLabel->setBrush(QBrush(Qt::black));
    presLabel->setPos(815, 255);
    presLabel->setZValue(10);
    m_scene->addItem(presLabel);

    QGraphicsRectItem *presBg = new QGraphicsRectItem(880, 252, 90, 22);
    presBg->setBrush(QBrush(Qt::black));
    presBg->setPen(Qt::NoPen);
    presBg->setZValue(10);
    m_scene->addItem(presBg);

    QGraphicsSimpleTextItem *presValue = new QGraphicsSimpleTextItem("8.6E-4");
    presValue->setFont(QFont("Courier New", 10));
    presValue->setBrush(QBrush(C_LED_GREEN));
    presValue->setPos(890, 257);
    presValue->setZValue(11);
    m_scene->addItem(presValue);

    // ==================== 底部控制按钮 ====================
    InteractiveButton *evacBtn = new InteractiveButton("EVACUATE");
    evacBtn->setPos(810, 100);
    evacBtn->setZValue(10);
    evacBtn->setCallback([this]() { onEvacuateClicked(); });
    m_scene->addItem(evacBtn);

    InteractiveButton *stop2Btn = new InteractiveButton("STOP");
    stop2Btn->setPos(810, 140);
    stop2Btn->setZValue(10);
    stop2Btn->setCallback([this]() { onStopClicked(); });
    m_scene->addItem(stop2Btn);

    InteractiveButton *ventBtn = new InteractiveButton("VENT");
    ventBtn->setPos(810, 180);
    ventBtn->setZValue(10);
    ventBtn->setCallback([this]() { onVentClicked(); });
    m_scene->addItem(ventBtn);

    // ==================== 底部状态面板 ====================
    QGraphicsRectItem *panel1 = new QGraphicsRectItem(10, 530, 380, 150);
    panel1->setBrush(QBrush(C_WIN_GRAY));
    panel1->setPen(QPen(Qt::darkGray, 1));
    panel1->setZValue(0);
    m_scene->addItem(panel1);

    QGraphicsSimpleTextItem *panel1Title = new QGraphicsSimpleTextItem("System power up lid down");
    panel1Title->setFont(QFont("Arial", 10, QFont::Bold));
    panel1Title->setBrush(QBrush(Qt::black));
    panel1Title->setPos(20, 540);
    panel1Title->setZValue(10);
    m_scene->addItem(panel1Title);

    // Lid
    QGraphicsSimpleTextItem *lidLabel = new QGraphicsSimpleTextItem("Lid:");
    lidLabel->setFont(QFont("Arial", 9));
    lidLabel->setBrush(QBrush(Qt::black));
    lidLabel->setPos(20, 570);
    lidLabel->setZValue(10);
    m_scene->addItem(lidLabel);

    QGraphicsRectItem *lidBox = new QGraphicsRectItem(55, 567, 80, 20);
    lidBox->setBrush(QBrush(C_WIN_LIGHT));
    lidBox->setPen(QPen(Qt::darkGray, 1));
    lidBox->setZValue(10);
    m_scene->addItem(lidBox);

    QGraphicsSimpleTextItem *lidValue = new QGraphicsSimpleTextItem("OPEN");
    lidValue->setFont(QFont("Arial", 9));
    lidValue->setBrush(QBrush(Qt::black));
    lidValue->setPos(70, 570);
    lidValue->setZValue(11);
    m_scene->addItem(lidValue);

    // Pirani
    QGraphicsSimpleTextItem *piraniLabel = new QGraphicsSimpleTextItem("Pirani:");
    piraniLabel->setFont(QFont("Arial", 9));
    piraniLabel->setBrush(QBrush(Qt::black));
    piraniLabel->setPos(20, 600);
    piraniLabel->setZValue(10);
    m_scene->addItem(piraniLabel);

    QGraphicsRectItem *piraniBox = new QGraphicsRectItem(65, 597, 90, 20);
    piraniBox->setBrush(QBrush(Qt::black));
    piraniBox->setPen(Qt::NoPen);
    piraniBox->setZValue(10);
    m_scene->addItem(piraniBox);

    QGraphicsSimpleTextItem *piraniValue = new QGraphicsSimpleTextItem("8.6E-4 mbar");
    piraniValue->setFont(QFont("Courier New", 9));
    piraniValue->setBrush(QBrush(C_LED_GREEN));
    piraniValue->setPos(70, 600);
    piraniValue->setZValue(11);
    m_scene->addItem(piraniValue);

    // Vent Time
    QGraphicsSimpleTextItem *ventLabel = new QGraphicsSimpleTextItem("Vent Time Left:");
    ventLabel->setFont(QFont("Arial", 9));
    ventLabel->setBrush(QBrush(Qt::black));
    ventLabel->setPos(20, 630);
    ventLabel->setZValue(10);
    m_scene->addItem(ventLabel);

    QGraphicsRectItem *ventBox = new QGraphicsRectItem(110, 627, 60, 20);
    ventBox->setBrush(QBrush(C_WIN_LIGHT));
    ventBox->setPen(QPen(Qt::darkGray, 1));
    ventBox->setZValue(10);
    m_scene->addItem(ventBox);

    QGraphicsSimpleTextItem *ventValue = new QGraphicsSimpleTextItem("0 s");
    ventValue->setFont(QFont("Arial", 9));
    ventValue->setBrush(QBrush(Qt::black));
    ventValue->setPos(120, 630);
    ventValue->setZValue(11);
    m_scene->addItem(ventValue);

    // ==================== 图例面板 ====================
    QGraphicsRectItem *legendPanel = new QGraphicsRectItem(410, 530, 380, 150);
    legendPanel->setBrush(QBrush(C_WIN_GRAY));
    legendPanel->setPen(QPen(Qt::darkGray, 1));
    legendPanel->setZValue(0);
    m_scene->addItem(legendPanel);

    QGraphicsSimpleTextItem *legendTitle = new QGraphicsSimpleTextItem("Legend:");
    legendTitle->setFont(QFont("Arial", 10, QFont::Bold));
    legendTitle->setBrush(QBrush(Qt::black));
    legendTitle->setPos(420, 540);
    legendTitle->setZValue(10);
    m_scene->addItem(legendTitle);

    // 第一列
    LEDIndicator *vOpenLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN);
    vOpenLed->setPos(420, 570);
    vOpenLed->setZValue(10);
    m_scene->addItem(vOpenLed);

    QGraphicsSimpleTextItem *vOpenLabel = new QGraphicsSimpleTextItem("Valve Open");
    vOpenLabel->setFont(QFont("Arial", 9));
    vOpenLabel->setBrush(QBrush(Qt::black));
    vOpenLabel->setPos(435, 573);
    vOpenLabel->setZValue(10);
    m_scene->addItem(vOpenLabel);

    LEDIndicator *vClosedLed = new LEDIndicator(true, C_LED_RED, C_LED_RED);
    vClosedLed->setPos(420, 600);
    vClosedLed->setZValue(10);
    m_scene->addItem(vClosedLed);

    QGraphicsSimpleTextItem *vClosedLabel = new QGraphicsSimpleTextItem("Valve Closed");
    vClosedLabel->setFont(QFont("Arial", 9));
    vClosedLabel->setBrush(QBrush(Qt::black));
    vClosedLabel->setPos(435, 603);
    vClosedLabel->setZValue(10);
    m_scene->addItem(vClosedLabel);

    LEDIndicator *pumpRunLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN);
    pumpRunLed->setPos(420, 630);
    pumpRunLed->setZValue(10);
    m_scene->addItem(pumpRunLed);

    QGraphicsSimpleTextItem *pumpRunLabel = new QGraphicsSimpleTextItem("Pump Running");
    pumpRunLabel->setFont(QFont("Arial", 9));
    pumpRunLabel->setBrush(QBrush(Qt::black));
    pumpRunLabel->setPos(435, 633);
    pumpRunLabel->setZValue(10);
    m_scene->addItem(pumpRunLabel);

    // 第二列
    LEDIndicator *procActLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN);
    procActLed->setPos(590, 570);
    procActLed->setZValue(10);
    m_scene->addItem(procActLed);

    QGraphicsSimpleTextItem *procActLabel = new QGraphicsSimpleTextItem("Process Active");
    procActLabel->setFont(QFont("Arial", 9));
    procActLabel->setBrush(QBrush(Qt::black));
    procActLabel->setPos(605, 573);
    procActLabel->setZValue(10);
    m_scene->addItem(procActLabel);

    LEDIndicator *intOkLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN);
    intOkLed->setPos(590, 600);
    intOkLed->setZValue(10);
    m_scene->addItem(intOkLed);

    QGraphicsSimpleTextItem *intOkLabel = new QGraphicsSimpleTextItem("Interlock OK");
    intOkLabel->setFont(QFont("Arial", 9));
    intOkLabel->setBrush(QBrush(Qt::black));
    intOkLabel->setPos(605, 603);
    intOkLabel->setZValue(10);
    m_scene->addItem(intOkLabel);

    LEDIndicator *intFaultLed = new LEDIndicator(true, C_LED_RED, C_LED_RED);
    intFaultLed->setPos(590, 630);
    intFaultLed->setZValue(10);
    m_scene->addItem(intFaultLed);

    QGraphicsSimpleTextItem *intFaultLabel = new QGraphicsSimpleTextItem("Interlock Fault");
    intFaultLabel->setFont(QFont("Arial", 9));
    intFaultLabel->setBrush(QBrush(Qt::black));
    intFaultLabel->setPos(605, 633);
    intFaultLabel->setZValue(10);
    m_scene->addItem(intFaultLabel);

    // 第三列
    LEDIndicator *waterFaultLed = new LEDIndicator(true, C_LED_RED, C_LED_RED);
    waterFaultLed->setPos(720, 570);
    waterFaultLed->setZValue(10);
    m_scene->addItem(waterFaultLed);

    QGraphicsSimpleTextItem *waterFaultLabel = new QGraphicsSimpleTextItem("Water Fault");
    waterFaultLabel->setFont(QFont("Arial", 9));
    waterFaultLabel->setBrush(QBrush(Qt::black));
    waterFaultLabel->setPos(735, 573);
    waterFaultLabel->setZValue(10);
    m_scene->addItem(waterFaultLabel);
}

// 菜单切换处理
void HardwareDiagram::onSystemMenuToggle()
{
    closeAllMenus();
    m_systemMenu->setVisible(true);
    m_systemBtn->setMenuVisible(true);
}

void HardwareDiagram::onProcessMenuToggle()
{
    closeAllMenus();
    m_processMenu->setVisible(true);
    m_processBtn->setMenuVisible(true);
}

void HardwareDiagram::onManagerMenuToggle()
{
    closeAllMenus();
    m_managerMenu->setVisible(true);
    m_managerBtn->setMenuVisible(true);
}

void HardwareDiagram::closeAllMenus()
{
    m_systemMenu->setVisible(false);
    m_processMenu->setVisible(false);
    m_managerMenu->setVisible(false);
    m_systemBtn->setMenuVisible(false);
    m_processBtn->setMenuVisible(false);
    m_managerBtn->setMenuVisible(false);
}

// 添加管道线
void HardwareDiagram::addPipeLine(qreal x1, qreal y1, qreal x2, qreal y2, const QPen &pen)
{
    QGraphicsLineItem *pipe = new QGraphicsLineItem(x1, y1, x2, y2);
    pipe->setPen(pen);
    pipe->setZValue(2);
    m_scene->addItem(pipe);
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
    PipeItem *pipe = dynamic_cast<PipeItem*>(m_components.value(name, nullptr));
    if (pipe) {
        pipe->setFlowDirection(flowing);
    }
}

HardwareComponent* HardwareDiagram::getComponent(const QString &name) const
{
    return m_components.value(name, nullptr);
}

// 按钮动作
void HardwareDiagram::onEvacuateClicked()
{
    qDebug() << "EVACUATE clicked!";
    emit evacuateRequested();
}

void HardwareDiagram::onStopClicked()
{
    qDebug() << "STOP clicked!";
    emit stopRequested();
}

void HardwareDiagram::onVentClicked()
{
    qDebug() << "VENT clicked!";
    emit ventRequested();
}
