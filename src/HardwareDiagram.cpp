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
    QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem("1", nullptr);
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
    QGraphicsSimpleTextItem *pLabel = new QGraphicsSimpleTextItem("P", nullptr);
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

    QGraphicsSimpleTextItem *freq = new QGraphicsSimpleTextItem("13.56", nullptr);
    freq->setFont(QFont("Arial", 11, QFont::Bold));
    freq->setBrush(QBrush(Qt::white));
    freq->setPos(-22, -18);
    addToGroup(freq);

    QGraphicsSimpleTextItem *mhz = new QGraphicsSimpleTextItem("MHz", nullptr);
    mhz->setFont(QFont("Arial", 7));
    mhz->setBrush(QBrush(Qt::white));
    mhz->setPos(-15, 0);
    addToGroup(mhz);

    QGraphicsSimpleTextItem *rfLabel = new QGraphicsSimpleTextItem("RF", nullptr);
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

    QGraphicsSimpleTextItem *freq = new QGraphicsSimpleTextItem("2", nullptr);
    freq->setFont(QFont("Arial", 18, QFont::Bold));
    freq->setBrush(QBrush(Qt::white));
    freq->setPos(-8, -22);
    addToGroup(freq);

    QGraphicsSimpleTextItem *mhz = new QGraphicsSimpleTextItem("MHz", nullptr);
    mhz->setFont(QFont("Arial", 8));
    mhz->setBrush(QBrush(Qt::white));
    mhz->setPos(-15, 2);
    addToGroup(mhz);

    QGraphicsSimpleTextItem *icpLabel = new QGraphicsSimpleTextItem("ICP", nullptr);
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

    QGraphicsSimpleTextItem *unit = new QGraphicsSimpleTextItem("mbar", nullptr);
    unit->setFont(QFont("Arial", 7));
    unit->setBrush(QBrush(QColor(180, 180, 180)));
    unit->setPos(-12, 0);
    addToGroup(unit);

    QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem("Pirani", nullptr);
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

    QGraphicsSimpleTextItem *unit = new QGraphicsSimpleTextItem("C", nullptr);
    unit->setFont(QFont("Arial", 10));
    unit->setBrush(QBrush(Qt::white));
    unit->setPos(12, -8);
    addToGroup(unit);

    QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem("Chuck", nullptr);
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
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    m_scene->setSceneRect(0, 0, 1100, 680);
    setupLayout();
}

HardwareDiagram::~HardwareDiagram()
{
}

// ==================== Pump Control Page - 参照原厂界面 ====================
void HardwareDiagram::setupLayout()
{
    m_scene->clear();
    m_components.clear();

    // 布局常量
    const int TOP_MENU_H = 40;          // 顶部菜单栏高度
    const int CONTROL_BAR_H = 45;       // 控制按钮栏高度
    const int MIMIC_START_Y = TOP_MENU_H + CONTROL_BAR_H; // 85
    const int ROBOT_BANNER_H = 30;      // Robot状态栏高度
    const int MIMIC_H = 320;            // Mimic区域高度
    const int PANEL_H = 150;            // 底部面板高度
    const int PANEL_Y = MIMIC_START_Y + ROBOT_BANNER_H + MIMIC_H + 10; // 545

    // ==================== 顶部菜单栏 ====================
    QGraphicsRectItem *menuBar = new QGraphicsRectItem(0, 0, 1100, TOP_MENU_H);
    menuBar->setBrush(QBrush(C_WIN_GRAY));
    menuBar->setPen(QPen(C_WIN_DARK, 1));
    menuBar->setZValue(0);
    m_scene->addItem(menuBar);

    // 页面标题 "Pump control page"
    QGraphicsSimpleTextItem *pageTitle = new QGraphicsSimpleTextItem("Pump control page");
    pageTitle->setFont(QFont("Arial", 11, QFont::Bold));
    pageTitle->setBrush(QBrush(Qt::black));
    pageTitle->setPos(10, 14);
    pageTitle->setZValue(10);
    m_scene->addItem(pageTitle);

    // System 按钮
    m_systemBtn = new MenuButton("System");
    m_systemBtn->setPos(180, 10);
    m_systemBtn->setZValue(10);
    m_scene->addItem(m_systemBtn);

    // System 下拉菜单
    m_systemMenu = new MenuPanel();
    m_systemMenu->setPos(180, 22);
    m_systemMenu->addMenuItem("Pumping", 1, nullptr);
    m_systemMenu->addMenuItem("Venting", 2, nullptr);
    m_systemMenu->addMenuItem("Configuration", 3, nullptr);
    m_systemMenu->addMenuItem("Service", 4, nullptr);
    m_systemMenu->addMenuItem("Passwords", 5, nullptr);
    m_systemMenu->addMenuItem("Status...", 6, nullptr);
    m_systemMenu->setVisible(false);
    m_scene->addItem(m_systemMenu);

    // Process 按钮 (带紫色活动指示器)
    m_processBtn = new MenuButton("Process");
    m_processBtn->setPos(285, 10);
    m_processBtn->setZValue(10);
    m_scene->addItem(m_processBtn);

    // Process 下拉菜单
    m_processMenu = new MenuPanel();
    m_processMenu->setPos(285, 22);
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
    m_managerBtn->setPos(390, 10);
    m_managerBtn->setZValue(10);
    m_scene->addItem(m_managerBtn);

    // Manager 下拉菜单
    m_managerMenu = new MenuPanel();
    m_managerMenu->setPos(390, 22);
    m_managerMenu->addMenuItem("Log on", 20, nullptr);
    m_managerMenu->addMenuItem("Log off", 21, nullptr);
    m_managerMenu->addMenuItem("Production Mode", 22, nullptr);
    m_managerMenu->addMenuItem("Edit Users", 23, nullptr);
    m_managerMenu->addMenuItem("Preferences", 24, nullptr);
    m_managerMenu->setVisible(false);
    m_scene->addItem(m_managerMenu);

    // PUMP CONTROL 标签
    QGraphicsSimpleTextItem *pumpLabel = new QGraphicsSimpleTextItem("PUMP CONTROL");
    pumpLabel->setFont(QFont("Arial", 10, QFont::Bold));
    pumpLabel->setBrush(QBrush(Qt::black));
    pumpLabel->setPos(510, 14);
    pumpLabel->setZValue(10);
    m_scene->addItem(pumpLabel);

    // LOG 圆形指示器
    QGraphicsEllipseItem *logGauge = new QGraphicsEllipseItem(640, 8, 26, 26);
    logGauge->setBrush(QBrush(C_WIN_GRAY));
    logGauge->setPen(QPen(Qt::darkGray, 2));
    logGauge->setZValue(10);
    m_scene->addItem(logGauge);

    QGraphicsSimpleTextItem *logLabel = new QGraphicsSimpleTextItem("LOG");
    logLabel->setFont(QFont("Arial", 8));
    logLabel->setBrush(QBrush(Qt::black));
    logLabel->setPos(647, 17);
    logLabel->setZValue(11);
    m_scene->addItem(logLabel);

    // 右侧 STOP ALL AUTO PROCESSES
    QGraphicsRectItem *stopAllBtn = new QGraphicsRectItem(700, 8, 180, 28);
    stopAllBtn->setBrush(QBrush(C_LED_GREEN));
    stopAllBtn->setPen(QPen(Qt::darkGray, 1));
    stopAllBtn->setZValue(10);
    m_scene->addItem(stopAllBtn);

    QGraphicsSimpleTextItem *stopAllText = new QGraphicsSimpleTextItem("STOP ALL AUTO PROCESSES");
    stopAllText->setFont(QFont("Arial", 9, QFont::Bold));
    stopAllText->setBrush(QBrush(Qt::white));
    stopAllText->setPos(710, 14);
    stopAllText->setZValue(11);
    m_scene->addItem(stopAllText);

    // STOP 六边形按钮
    QGraphicsPolygonItem *stopHex = new QGraphicsPolygonItem();
    QPolygonF hex;
    hex << QPointF(920, 5) << QPointF(940, 15) << QPointF(940, 30) << QPointF(920, 40) << QPointF(900, 30) << QPointF(900, 15);
    stopHex->setPolygon(hex);
    stopHex->setBrush(QBrush(C_LED_RED));
    stopHex->setPen(QPen(Qt::darkGray, 1));
    stopHex->setZValue(10);
    m_scene->addItem(stopHex);

    QGraphicsSimpleTextItem *stopLabel = new QGraphicsSimpleTextItem("STOP");
    stopLabel->setFont(QFont("Arial", 10, QFont::Bold));
    stopLabel->setBrush(QBrush(Qt::white));
    stopLabel->setPos(905, 18);
    stopLabel->setZValue(11);
    m_scene->addItem(stopLabel);

    // 设置菜单按钮回调
    m_systemBtn->setCallback([this]() { onSystemMenuToggle(); });
    m_processBtn->setCallback([this]() { onProcessMenuToggle(); });
    m_managerBtn->setCallback([this]() { onManagerMenuToggle(); });

    // ==================== Robot Arm 状态栏 ====================
    QGraphicsRectItem *robotBanner = new QGraphicsRectItem(0, MIMIC_START_Y, 1100, ROBOT_BANNER_H);
    robotBanner->setBrush(QBrush(C_TITLE_BLUE));
    robotBanner->setPen(QPen(C_WIN_DARK, 1));
    robotBanner->setZValue(0);
    m_scene->addItem(robotBanner);

    QGraphicsSimpleTextItem *robotStatus = new QGraphicsSimpleTextItem("Robot arm standing by");
    robotStatus->setFont(QFont("Arial", 10));
    robotStatus->setBrush(QBrush(Qt::white));
    robotStatus->setPos(10, MIMIC_START_Y + 8);
    robotStatus->setZValue(10);
    m_scene->addItem(robotStatus);

    // ==================== Mimic 背景 ====================
    QGraphicsRectItem *mimicBg = new QGraphicsRectItem(0, MIMIC_START_Y + ROBOT_BANNER_H, 1100, MIMIC_H);
    mimicBg->setBrush(QBrush(C_MENU_BG));
    mimicBg->setPen(QPen(Qt::darkGray, 1));
    mimicBg->setZValue(0);
    m_scene->addItem(mimicBg);

    // Mimic 内部布局常量
    const int INNER_Y = MIMIC_START_Y + ROBOT_BANNER_H + 10;
    const int CHAMBER_CX = 500;   // 主腔室中心X
    const int CHAMBER_CY = INNER_Y + 160;  // 主腔室中心Y

    // ==================== 绘制管道 ====================
    QPen pipePen(C_PIPE_BLACK, 4);

    // 主腔室到机械泵 (MP)
    addPipeLine(CHAMBER_CX, CHAMBER_CY + 60, CHAMBER_CX, CHAMBER_CY + 100, pipePen);
    addPipeLine(CHAMBER_CX, CHAMBER_CY + 100, 150, CHAMBER_CY + 100, pipePen);
    addPipeLine(150, CHAMBER_CY + 100, 150, INNER_Y + 290, pipePen);

    // 主腔室到分子泵 (TP)
    addPipeLine(CHAMBER_CX + 50, CHAMBER_CY + 60, CHAMBER_CX + 50, CHAMBER_CY + 100, pipePen);
    addPipeLine(CHAMBER_CX + 50, CHAMBER_CY + 100, 850, CHAMBER_CY + 100, pipePen);
    addPipeLine(850, CHAMBER_CY + 100, 850, INNER_Y + 290, pipePen);

    // Load-Lock 到 MP
    addPipeLine(200, INNER_Y + 80, 200, INNER_Y + 120, pipePen);
    addPipeLine(200, INNER_Y + 120, 150, INNER_Y + 120, pipePen);
    addPipeLine(150, INNER_Y + 120, 150, INNER_Y + 180, pipePen);

    // Load-Lock 门阀连接
    addPipeLine(280, INNER_Y + 80, CHAMBER_CX - 80, INNER_Y + 80, pipePen);
    addPipeLine(CHAMBER_CX - 80, INNER_Y + 80, CHAMBER_CX - 80, CHAMBER_CY - 60, pipePen);

    // RF 管道
    addPipeLine(CHAMBER_CX - 80, CHAMBER_CY, 320, CHAMBER_CY, pipePen);

    // ICP 管道
    addPipeLine(CHAMBER_CX, CHAMBER_CY - 80, CHAMBER_CX, INNER_Y + 50, pipePen);

    // 压力计连接
    addPipeLine(CHAMBER_CX + 80, CHAMBER_CY, 950, CHAMBER_CY, pipePen);

    // 气体管道
    addPipeLine(80, CHAMBER_CY - 40, 80, CHAMBER_CY + 40, pipePen);
    addPipeLine(80, CHAMBER_CY + 40, CHAMBER_CX - 40, CHAMBER_CY + 40, pipePen);
    addPipeLine(CHAMBER_CX - 40, CHAMBER_CY + 40, CHAMBER_CX - 40, CHAMBER_CY + 10, pipePen);

    // ==================== Load-Lock 区域 (左侧) ====================
    // Load-Lock 腔室 (较小)
    QGraphicsEllipseItem *loadLockOuter = new QGraphicsEllipseItem(160, INNER_Y + 30, 80, 80);
    loadLockOuter->setBrush(QBrush(C_CHAMBER_OUTER));
    loadLockOuter->setPen(QPen(Qt::darkGray, 2));
    loadLockOuter->setZValue(5);
    m_scene->addItem(loadLockOuter);

    QGraphicsEllipseItem *loadLockInner = new QGraphicsEllipseItem(170, INNER_Y + 40, 60, 60);
    loadLockInner->setBrush(QBrush(C_CHAMBER_INNER));
    loadLockInner->setPen(Qt::NoPen);
    loadLockInner->setZValue(5);
    m_scene->addItem(loadLockInner);

    QGraphicsSimpleTextItem *llLabel = new QGraphicsSimpleTextItem("Load Lock", nullptr);
    llLabel->setFont(QFont("Arial", 8));
    llLabel->setBrush(QBrush(Qt::black));
    llLabel->setPos(175, INNER_Y + 60);
    llLabel->setZValue(6);
    m_scene->addItem(llLabel);

    // Load-Lock 阀门 V_LL
    ValveItem *vLL = new ValveItem("V_LL");
    vLL->setPos(200, INNER_Y + 120);
    vLL->setOpen(false);
    vLL->setZValue(5);
    m_scene->addItem(vLL);
    m_components["V_LL"] = vLL;

    // ==================== 主腔室 (中部) ====================
    ChamberItem *chamber = new ChamberItem("Chamber");
    chamber->setPos(CHAMBER_CX, CHAMBER_CY);
    chamber->setState(ComponentState::Normal);
    chamber->setZValue(5);
    m_scene->addItem(chamber);
    m_components["Chamber"] = chamber;

    // Chuck (样品架)
    ChuckItem *chuck = new ChuckItem("Chuck");
    chuck->setPos(CHAMBER_CX, CHAMBER_CY + 90);
    chuck->setState(ComponentState::Normal);
    chuck->setZValue(5);
    m_scene->addItem(chuck);
    m_components["Chuck"] = chuck;

    // ==================== RF 发生器 (左侧) ====================
    RFGeneratorItem *rf = new RFGeneratorItem("RF Generator");
    rf->setPos(290, CHAMBER_CY);
    rf->setState(ComponentState::Normal);
    rf->setZValue(5);
    m_scene->addItem(rf);
    m_components["RF"] = rf;

    // ==================== ICP 源 (上方) ====================
    ICPGeneratorItem *icp = new ICPGeneratorItem("ICP Source");
    icp->setPos(CHAMBER_CX, INNER_Y + 30);
    icp->setState(ComponentState::Normal);
    icp->setZValue(5);
    m_scene->addItem(icp);
    m_components["ICP"] = icp;

    // ==================== 气体 MFC (最左侧) ====================
    QStringList gases = {"Ar", "O2", "N2", "CF4"};
    for (int i = 0; i < 4; i++) {
        GasMFCItem *mfc = new GasMFCItem(gases[i], i+1);
        mfc->setPos(60, INNER_Y + 80 + i * 55);
        mfc->setState(ComponentState::Normal);
        mfc->setZValue(5);
        m_scene->addItem(mfc);
        m_components[gases[i]] = mfc;
    }

    // 气体入口阀门 V_GAS
    ValveItem *vGas = new ValveItem("V_GAS");
    vGas->setPos(80, CHAMBER_CY);
    vGas->setOpen(false);
    vGas->setZValue(5);
    m_scene->addItem(vGas);
    m_components["V_GAS"] = vGas;

    // ==================== 泵 (底部) ====================
    // MP 机械泵
    PumpItem *mp = new PumpItem("MP");
    mp->setPos(150, INNER_Y + 300);
    mp->setState(ComponentState::Normal);
    mp->setZValue(5);
    m_scene->addItem(mp);
    m_components["MP"] = mp;

    // TP 分子泵
    PumpItem *tp = new PumpItem("TP");
    tp->setPos(850, INNER_Y + 300);
    tp->setState(ComponentState::Normal);
    tp->setZValue(5);
    m_scene->addItem(tp);
    m_components["TP"] = tp;

    // ==================== 阀门 (主管道上) ====================
    ValveItem *v1 = new ValveItem("V1");  // 主腔室到MP
    v1->setPos(CHAMBER_CX, CHAMBER_CY + 80);
    v1->setOpen(false);
    v1->setZValue(5);
    m_scene->addItem(v1);
    m_components["V1"] = v1;

    ValveItem *v2 = new ValveItem("V2");  // 主腔室到TP
    v2->setPos(CHAMBER_CX + 50, CHAMBER_CY + 80);
    v2->setOpen(false);
    v2->setZValue(5);
    m_scene->addItem(v2);
    m_components["V2"] = v2;

    // ==================== 压力计 (右侧) ====================
    PressureGaugeItem *pressure = new PressureGaugeItem("Pressure");
    pressure->setPos(980, CHAMBER_CY);
    pressure->setState(ComponentState::Normal);
    pressure->setZValue(5);
    m_scene->addItem(pressure);
    m_components["Pressure"] = pressure;

    // Pirani 规
    PressureGaugeItem *pirani = new PressureGaugeItem("Pirani");
    pirani->setPos(980, INNER_Y + 100);
    pirani->setState(ComponentState::Normal);
    pirani->setZValue(5);
    m_scene->addItem(pirani);
    m_components["Pirani"] = pirani;

    // ==================== 状态面板 (右侧中部) ====================
    const int STATUS_PANEL_X = 1000;
    const int STATUS_PANEL_Y = INNER_Y + 10;
    const int STATUS_PANEL_W = 90;
    const int STATUS_PANEL_H = 120;

    QGraphicsRectItem *statusPanel = new QGraphicsRectItem(STATUS_PANEL_X, STATUS_PANEL_Y, STATUS_PANEL_W, STATUS_PANEL_H);
    statusPanel->setBrush(QBrush(C_WIN_GRAY));
    statusPanel->setPen(QPen(Qt::darkGray, 1));
    statusPanel->setZValue(0);
    m_scene->addItem(statusPanel);

    // Turbo Pump Status
    QGraphicsSimpleTextItem *turboLabel = new QGraphicsSimpleTextItem("Turbo", nullptr);
    turboLabel->setFont(QFont("Arial", 8));
    turboLabel->setBrush(QBrush(Qt::black));
    turboLabel->setPos(STATUS_PANEL_X + 5, STATUS_PANEL_Y + 5);
    turboLabel->setZValue(10);
    m_scene->addItem(turboLabel);

    LEDIndicator *turboLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN_DARK);
    turboLed->setPos(STATUS_PANEL_X + 5, STATUS_PANEL_Y + 25);
    turboLed->setZValue(10);
    m_scene->addItem(turboLed);

    QGraphicsSimpleTextItem *atSpeedLabel = new QGraphicsSimpleTextItem("at speed", nullptr);
    atSpeedLabel->setFont(QFont("Arial", 8));
    atSpeedLabel->setBrush(QBrush(C_LED_GREEN));
    atSpeedLabel->setPos(STATUS_PANEL_X + 20, STATUS_PANEL_Y + 28);
    atSpeedLabel->setZValue(10);
    m_scene->addItem(atSpeedLabel);

    // N2 Flow Rate
    QGraphicsSimpleTextItem *n2Label = new QGraphicsSimpleTextItem("N2:", nullptr);
    n2Label->setFont(QFont("Arial", 8));
    n2Label->setBrush(QBrush(Qt::black));
    n2Label->setPos(STATUS_PANEL_X + 5, STATUS_PANEL_Y + 55);
    n2Label->setZValue(10);
    m_scene->addItem(n2Label);

    QGraphicsRectItem *n2Box = new QGraphicsRectItem(STATUS_PANEL_X + 5, STATUS_PANEL_Y + 72, 80, 18);
    n2Box->setBrush(QBrush(Qt::black));
    n2Box->setPen(Qt::NoPen);
    n2Box->setZValue(10);
    m_scene->addItem(n2Box);

    QGraphicsSimpleTextItem *n2Value = new QGraphicsSimpleTextItem("0.0 sccm", nullptr);
    n2Value->setFont(QFont("Courier New", 9));
    n2Value->setBrush(QBrush(C_LED_GREEN));
    n2Value->setPos(STATUS_PANEL_X + 10, STATUS_PANEL_Y + 75);
    n2Value->setZValue(11);
    m_scene->addItem(n2Value);

    // Set Base Pressure 按钮
    InteractiveButton *setBaseBtn = new InteractiveButton("Set Base");
    setBaseBtn->setPos(STATUS_PANEL_X + 5, STATUS_PANEL_Y + 95);
    setBaseBtn->setZValue(10);
    m_scene->addItem(setBaseBtn);

    // ==================== 左侧状态面板 (Robot Arm Status) ====================
    const int ARM_PANEL_X = 10;
    const int ARM_PANEL_Y = INNER_Y + 10;
    const int ARM_PANEL_W = 130;
    const int ARM_PANEL_H = 100;

    QGraphicsRectItem *armPanel = new QGraphicsRectItem(ARM_PANEL_X, ARM_PANEL_Y, ARM_PANEL_W, ARM_PANEL_H);
    armPanel->setBrush(QBrush(C_WIN_GRAY));
    armPanel->setPen(QPen(Qt::darkGray, 1));
    armPanel->setZValue(0);
    m_scene->addItem(armPanel);

    QGraphicsSimpleTextItem *armTitle = new QGraphicsSimpleTextItem("Robot Arm", nullptr);
    armTitle->setFont(QFont("Arial", 9, QFont::Bold));
    armTitle->setBrush(QBrush(Qt::black));
    armTitle->setPos(ARM_PANEL_X + 5, ARM_PANEL_Y + 5);
    armTitle->setZValue(10);
    m_scene->addItem(armTitle);

    // ARM HOME (绿色)
    LEDIndicator *armHomeLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN_DARK);
    armHomeLed->setPos(ARM_PANEL_X + 5, ARM_PANEL_Y + 28);
    armHomeLed->setZValue(10);
    m_scene->addItem(armHomeLed);
    QGraphicsSimpleTextItem *armHomeLbl = new QGraphicsSimpleTextItem("ARM HOME", nullptr);
    armHomeLbl->setFont(QFont("Arial", 8));
    armHomeLbl->setBrush(QBrush(Qt::black));
    armHomeLbl->setPos(ARM_PANEL_X + 20, ARM_PANEL_Y + 30);
    armHomeLbl->setZValue(10);
    m_scene->addItem(armHomeLbl);

    // ARM EXTENDED (黑色/关闭)
    LEDIndicator *armExtLed = new LEDIndicator(false, C_LED_GREEN, C_WIN_DARK);
    armExtLed->setPos(ARM_PANEL_X + 5, ARM_PANEL_Y + 48);
    armExtLed->setZValue(10);
    m_scene->addItem(armExtLed);
    QGraphicsSimpleTextItem *armExtLbl = new QGraphicsSimpleTextItem("ARM EXTENDED", nullptr);
    armExtLbl->setFont(QFont("Arial", 8));
    armExtLbl->setBrush(QBrush(Qt::black));
    armExtLbl->setPos(ARM_PANEL_X + 20, ARM_PANEL_Y + 50);
    armExtLbl->setZValue(10);
    m_scene->addItem(armExtLbl);

    // ARM FAULT (黑色/关闭)
    LEDIndicator *armFaultLed = new LEDIndicator(false, C_LED_RED, C_WIN_DARK);
    armFaultLed->setPos(ARM_PANEL_X + 5, ARM_PANEL_Y + 68);
    armFaultLed->setZValue(10);
    m_scene->addItem(armFaultLed);
    QGraphicsSimpleTextItem *armFaultLbl = new QGraphicsSimpleTextItem("ARM FAULT", nullptr);
    armFaultLbl->setFont(QFont("Arial", 8));
    armFaultLbl->setBrush(QBrush(Qt::black));
    armFaultLbl->setPos(ARM_PANEL_X + 20, ARM_PANEL_Y + 70);
    armFaultLbl->setZValue(10);
    m_scene->addItem(armFaultLbl);

    // WAFER LIFT / WAFER CLAMP
    QGraphicsSimpleTextItem *waferLiftLbl = new QGraphicsSimpleTextItem("WAFER LIFT DOWN", nullptr);
    waferLiftLbl->setFont(QFont("Arial", 7));
    waferLiftLbl->setBrush(QBrush(Qt::black));
    waferLiftLbl->setPos(ARM_PANEL_X + 5, ARM_PANEL_Y + 88);
    waferLiftLbl->setZValue(10);
    m_scene->addItem(waferLiftLbl);

    // ==================== 底部面板 - Load-Lock 区域 (左侧) ====================
    QGraphicsRectItem *llPanel = new QGraphicsRectItem(10, PANEL_Y, 340, PANEL_H);
    llPanel->setBrush(QBrush(C_WIN_GRAY));
    llPanel->setPen(QPen(Qt::darkGray, 1));
    llPanel->setZValue(0);
    m_scene->addItem(llPanel);

    QGraphicsSimpleTextItem *llPanelTitle = new QGraphicsSimpleTextItem("Load-Lock Area", nullptr);
    llPanelTitle->setFont(QFont("Arial", 10, QFont::Bold));
    llPanelTitle->setBrush(QBrush(Qt::black));
    llPanelTitle->setPos(20, PANEL_Y + 10);
    llPanelTitle->setZValue(10);
    m_scene->addItem(llPanelTitle);

    // Status
    QGraphicsSimpleTextItem *llStatusLbl = new QGraphicsSimpleTextItem("Status:", nullptr);
    llStatusLbl->setFont(QFont("Arial", 9));
    llStatusLbl->setBrush(QBrush(Qt::black));
    llStatusLbl->setPos(20, PANEL_Y + 35);
    llStatusLbl->setZValue(10);
    m_scene->addItem(llStatusLbl);

    QGraphicsSimpleTextItem *llStatusValue = new QGraphicsSimpleTextItem("Stopped Pumping/Venting", nullptr);
    llStatusValue->setFont(QFont("Arial", 9, QFont::Bold));
    llStatusValue->setBrush(QBrush(C_LED_GREEN));
    llStatusValue->setPos(70, PANEL_Y + 35);
    llStatusValue->setZValue(10);
    m_scene->addItem(llStatusValue);

    // Lid
    QGraphicsSimpleTextItem *llLidLbl = new QGraphicsSimpleTextItem("Lid:", nullptr);
    llLidLbl->setFont(QFont("Arial", 9));
    llLidLbl->setBrush(QBrush(Qt::black));
    llLidLbl->setPos(20, PANEL_Y + 60);
    llLidLbl->setZValue(10);
    m_scene->addItem(llLidLbl);

    QGraphicsRectItem *llLidBox = new QGraphicsRectItem(60, PANEL_Y + 57, 100, 20);
    llLidBox->setBrush(QBrush(C_WIN_LIGHT));
    llLidBox->setPen(QPen(Qt::darkGray, 1));
    llLidBox->setZValue(10);
    m_scene->addItem(llLidBox);

    QGraphicsSimpleTextItem *llLidValue = new QGraphicsSimpleTextItem("CLOSED", nullptr);
    llLidValue->setFont(QFont("Arial", 9));
    llLidValue->setBrush(QBrush(Qt::black));
    llLidValue->setPos(85, PANEL_Y + 60);
    llLidValue->setZValue(11);
    m_scene->addItem(llLidValue);

    // Pirani
    QGraphicsSimpleTextItem *llPiraniLbl = new QGraphicsSimpleTextItem("Pirani:", nullptr);
    llPiraniLbl->setFont(QFont("Arial", 9));
    llPiraniLbl->setBrush(QBrush(Qt::black));
    llPiraniLbl->setPos(20, PANEL_Y + 88);
    llPiraniLbl->setZValue(10);
    m_scene->addItem(llPiraniLbl);

    QGraphicsRectItem *llPiraniBox = new QGraphicsRectItem(70, PANEL_Y + 85, 110, 20);
    llPiraniBox->setBrush(QBrush(Qt::black));
    llPiraniBox->setPen(Qt::NoPen);
    llPiraniBox->setZValue(10);
    m_scene->addItem(llPiraniBox);

    QGraphicsSimpleTextItem *llPiraniValue = new QGraphicsSimpleTextItem("6.19e-05 Torr", nullptr);
    llPiraniValue->setFont(QFont("Courier New", 9));
    llPiraniValue->setBrush(QBrush(C_LED_GREEN));
    llPiraniValue->setPos(75, PANEL_Y + 88);
    llPiraniValue->setZValue(11);
    m_scene->addItem(llPiraniValue);

    // Vent Time Left
    QGraphicsSimpleTextItem *llVentLbl = new QGraphicsSimpleTextItem("Vent Time:", nullptr);
    llVentLbl->setFont(QFont("Arial", 9));
    llVentLbl->setBrush(QBrush(Qt::black));
    llVentLbl->setPos(20, PANEL_Y + 116);
    llVentLbl->setZValue(10);
    m_scene->addItem(llVentLbl);

    QGraphicsRectItem *llVentBox = new QGraphicsRectItem(95, PANEL_Y + 113, 70, 20);
    llVentBox->setBrush(QBrush(C_WIN_LIGHT));
    llVentBox->setPen(QPen(Qt::darkGray, 1));
    llVentBox->setZValue(10);
    m_scene->addItem(llVentBox);

    QGraphicsSimpleTextItem *llVentValue = new QGraphicsSimpleTextItem("0 secs", nullptr);
    llVentValue->setFont(QFont("Arial", 9));
    llVentValue->setBrush(QBrush(Qt::black));
    llVentValue->setPos(105, PANEL_Y + 116);
    llVentValue->setZValue(11);
    m_scene->addItem(llVentValue);

    // Load-Lock 按钮
    InteractiveButton *llEvacBtn = new InteractiveButton("EVACUATE");
    llEvacBtn->setPos(200, PANEL_Y + 50);
    llEvacBtn->setZValue(10);
    llEvacBtn->setCallback([this]() { onEvacuateClicked(); });
    m_scene->addItem(llEvacBtn);

    InteractiveButton *llStopBtn = new InteractiveButton("STOP");
    llStopBtn->setPos(200, PANEL_Y + 80);
    llStopBtn->setZValue(10);
    llStopBtn->setCallback([this]() { onStopClicked(); });
    m_scene->addItem(llStopBtn);

    InteractiveButton *llVentBtn = new InteractiveButton("VENT");
    llVentBtn->setPos(200, PANEL_Y + 110);
    llVentBtn->setZValue(10);
    llVentBtn->setCallback([this]() { onVentClicked(); });
    m_scene->addItem(llVentBtn);

    // ==================== 底部面板 - Process Chamber 区域 (右侧) ====================
    QGraphicsRectItem *procPanel = new QGraphicsRectItem(370, PANEL_Y, 340, PANEL_H);
    procPanel->setBrush(QBrush(C_WIN_GRAY));
    procPanel->setPen(QPen(Qt::darkGray, 1));
    procPanel->setZValue(0);
    m_scene->addItem(procPanel);

    QGraphicsSimpleTextItem *procPanelTitle = new QGraphicsSimpleTextItem("Process Chamber Area", nullptr);
    procPanelTitle->setFont(QFont("Arial", 10, QFont::Bold));
    procPanelTitle->setBrush(QBrush(Qt::black));
    procPanelTitle->setPos(380, PANEL_Y + 10);
    procPanelTitle->setZValue(10);
    m_scene->addItem(procPanelTitle);

    // Status
    QGraphicsSimpleTextItem *procStatusLbl = new QGraphicsSimpleTextItem("Status:", nullptr);
    procStatusLbl->setFont(QFont("Arial", 9));
    procStatusLbl->setBrush(QBrush(Qt::black));
    procStatusLbl->setPos(380, PANEL_Y + 35);
    procStatusLbl->setZValue(10);
    m_scene->addItem(procStatusLbl);

    QGraphicsSimpleTextItem *procStatusValue = new QGraphicsSimpleTextItem("Base pressure reached", nullptr);
    procStatusValue->setFont(QFont("Arial", 9, QFont::Bold));
    procStatusValue->setBrush(QBrush(C_LED_GREEN));
    procStatusValue->setPos(430, PANEL_Y + 35);
    procStatusValue->setZValue(10);
    m_scene->addItem(procStatusValue);

    // Lid
    QGraphicsSimpleTextItem *procLidLbl = new QGraphicsSimpleTextItem("Lid:", nullptr);
    procLidLbl->setFont(QFont("Arial", 9));
    procLidLbl->setBrush(QBrush(Qt::black));
    procLidLbl->setPos(380, PANEL_Y + 58);
    procLidLbl->setZValue(10);
    m_scene->addItem(procLidLbl);

    QGraphicsRectItem *procLidBox = new QGraphicsRectItem(420, PANEL_Y + 55, 100, 20);
    procLidBox->setBrush(QBrush(C_WIN_LIGHT));
    procLidBox->setPen(QPen(Qt::darkGray, 1));
    procLidBox->setZValue(10);
    m_scene->addItem(procLidBox);

    QGraphicsSimpleTextItem *procLidValue = new QGraphicsSimpleTextItem("CLOSED", nullptr);
    procLidValue->setFont(QFont("Arial", 9));
    procLidValue->setBrush(QBrush(Qt::black));
    procLidValue->setPos(445, PANEL_Y + 58);
    procLidValue->setZValue(11);
    m_scene->addItem(procLidValue);

    // Process Interlock
    QGraphicsSimpleTextItem *procIntLbl = new QGraphicsSimpleTextItem("Interlock:", nullptr);
    procIntLbl->setFont(QFont("Arial", 9));
    procIntLbl->setBrush(QBrush(Qt::black));
    procIntLbl->setPos(380, PANEL_Y + 83);
    procIntLbl->setZValue(10);
    m_scene->addItem(procIntLbl);

    LEDIndicator *procIntLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN_DARK);
    procIntLed->setPos(445, PANEL_Y + 78);
    procIntLed->setZValue(10);
    m_scene->addItem(procIntLed);

    QGraphicsSimpleTextItem *procIntValue = new QGraphicsSimpleTextItem("OK", nullptr);
    procIntValue->setFont(QFont("Arial", 9, QFont::Bold));
    procIntValue->setBrush(QBrush(C_LED_GREEN));
    procIntValue->setPos(460, PANEL_Y + 83);
    procIntValue->setZValue(10);
    m_scene->addItem(procIntValue);

    // Penning
    QGraphicsSimpleTextItem *penningLbl = new QGraphicsSimpleTextItem("Penning:", nullptr);
    penningLbl->setFont(QFont("Arial", 9));
    penningLbl->setBrush(QBrush(Qt::black));
    penningLbl->setPos(540, PANEL_Y + 58);
    penningLbl->setZValue(10);
    m_scene->addItem(penningLbl);

    QGraphicsRectItem *penningBox = new QGraphicsRectItem(595, PANEL_Y + 55, 100, 20);
    penningBox->setBrush(QBrush(Qt::black));
    penningBox->setPen(Qt::NoPen);
    penningBox->setZValue(10);
    m_scene->addItem(penningBox);

    QGraphicsSimpleTextItem *penningValue = new QGraphicsSimpleTextItem("3.50e-05 Torr", nullptr);
    penningValue->setFont(QFont("Courier New", 9));
    penningValue->setBrush(QBrush(C_LED_GREEN));
    penningValue->setPos(600, PANEL_Y + 58);
    penningValue->setZValue(11);
    m_scene->addItem(penningValue);

    // Cm Gauge
    QGraphicsSimpleTextItem *cmLbl = new QGraphicsSimpleTextItem("Cm:", nullptr);
    cmLbl->setFont(QFont("Arial", 9));
    cmLbl->setBrush(QBrush(Qt::black));
    cmLbl->setPos(540, PANEL_Y + 83);
    cmLbl->setZValue(10);
    m_scene->addItem(cmLbl);

    QGraphicsRectItem *cmBox = new QGraphicsRectItem(570, PANEL_Y + 80, 60, 20);
    cmBox->setBrush(QBrush(C_WIN_LIGHT));
    cmBox->setPen(QPen(Qt::darkGray, 1));
    cmBox->setZValue(10);
    m_scene->addItem(cmBox);

    QGraphicsSimpleTextItem *cmValue = new QGraphicsSimpleTextItem("2 mTorr", nullptr);
    cmValue->setFont(QFont("Arial", 9));
    cmValue->setBrush(QBrush(Qt::black));
    cmValue->setPos(575, PANEL_Y + 83);
    cmValue->setZValue(11);
    m_scene->addItem(cmValue);

    // Vent Time Left
    QGraphicsSimpleTextItem *procVentLbl = new QGraphicsSimpleTextItem("Vent Time:", nullptr);
    procVentLbl->setFont(QFont("Arial", 9));
    procVentLbl->setBrush(QBrush(Qt::black));
    procVentLbl->setPos(380, PANEL_Y + 116);
    procVentLbl->setZValue(10);
    m_scene->addItem(procVentLbl);

    QGraphicsRectItem *procVentBox = new QGraphicsRectItem(455, PANEL_Y + 113, 70, 20);
    procVentBox->setBrush(QBrush(C_WIN_LIGHT));
    procVentBox->setPen(QPen(Qt::darkGray, 1));
    procVentBox->setZValue(10);
    m_scene->addItem(procVentBox);

    QGraphicsSimpleTextItem *procVentValue = new QGraphicsSimpleTextItem("0 secs", nullptr);
    procVentValue->setFont(QFont("Arial", 9));
    procVentValue->setBrush(QBrush(Qt::black));
    procVentValue->setPos(465, PANEL_Y + 116);
    procVentValue->setZValue(11);
    m_scene->addItem(procVentValue);

    // Process Chamber 按钮
    InteractiveButton *procEvacBtn = new InteractiveButton("EVACUATE");
    procEvacBtn->setPos(560, PANEL_Y + 50);
    procEvacBtn->setZValue(10);
    procEvacBtn->setCallback([this]() { onEvacuateClicked(); });
    m_scene->addItem(procEvacBtn);

    InteractiveButton *procStopBtn = new InteractiveButton("STOP");
    procStopBtn->setPos(560, PANEL_Y + 80);
    procStopBtn->setZValue(10);
    procStopBtn->setCallback([this]() { onStopClicked(); });
    m_scene->addItem(procStopBtn);

    InteractiveButton *procVentBtn = new InteractiveButton("VENT");
    procVentBtn->setPos(560, PANEL_Y + 110);
    procVentBtn->setZValue(10);
    procVentBtn->setCallback([this]() { onVentClicked(); });
    m_scene->addItem(procVentBtn);

    // ==================== 底部右侧 - 另一部分面板 ====================
    QGraphicsRectItem *rightPanel = new QGraphicsRectItem(730, PANEL_Y, 360, PANEL_H);
    rightPanel->setBrush(QBrush(C_WIN_GRAY));
    rightPanel->setPen(QPen(Qt::darkGray, 1));
    rightPanel->setZValue(0);
    m_scene->addItem(rightPanel);

    QGraphicsSimpleTextItem *rightTitle = new QGraphicsSimpleTextItem("System Status", nullptr);
    rightTitle->setFont(QFont("Arial", 10, QFont::Bold));
    rightTitle->setBrush(QBrush(Qt::black));
    rightTitle->setPos(740, PANEL_Y + 10);
    rightTitle->setZValue(10);
    m_scene->addItem(rightTitle);

    // Connection Status
    QGraphicsSimpleTextItem *connLbl = new QGraphicsSimpleTextItem("Connection:", nullptr);
    connLbl->setFont(QFont("Arial", 9));
    connLbl->setBrush(QBrush(Qt::black));
    connLbl->setPos(740, PANEL_Y + 38);
    connLbl->setZValue(10);
    m_scene->addItem(connLbl);

    LEDIndicator *connLed = new LEDIndicator(false, C_LED_GREEN, C_LED_GREEN_DARK);
    connLed->setPos(820, PANEL_Y + 33);
    connLed->setZValue(10);
    m_scene->addItem(connLed);

    QGraphicsSimpleTextItem *connValue = new QGraphicsSimpleTextItem("Disconnected", nullptr);
    connValue->setFont(QFont("Arial", 9, QFont::Bold));
    connValue->setBrush(QBrush(C_LED_RED));
    connValue->setPos(840, PANEL_Y + 38);
    connValue->setZValue(10);
    m_scene->addItem(connValue);

    // Host
    QGraphicsSimpleTextItem *hostLbl = new QGraphicsSimpleTextItem("Host:", nullptr);
    hostLbl->setFont(QFont("Arial", 9));
    hostLbl->setBrush(QBrush(Qt::black));
    hostLbl->setPos(740, PANEL_Y + 63);
    hostLbl->setZValue(10);
    m_scene->addItem(hostLbl);

    QGraphicsRectItem *hostBox = new QGraphicsRectItem(780, PANEL_Y + 60, 140, 20);
    hostBox->setBrush(QBrush(C_WIN_LIGHT));
    hostBox->setPen(QPen(Qt::darkGray, 1));
    hostBox->setZValue(10);
    m_scene->addItem(hostBox);

    QGraphicsSimpleTextItem *hostValue = new QGraphicsSimpleTextItem("192.168.1.100", nullptr);
    hostValue->setFont(QFont("Arial", 9));
    hostValue->setBrush(QBrush(Qt::black));
    hostValue->setPos(785, PANEL_Y + 63);
    hostValue->setZValue(11);
    m_scene->addItem(hostValue);

    // Port
    QGraphicsSimpleTextItem *portLbl = new QGraphicsSimpleTextItem("Port:", nullptr);
    portLbl->setFont(QFont("Arial", 9));
    portLbl->setBrush(QBrush(Qt::black));
    portLbl->setPos(740, PANEL_Y + 90);
    portLbl->setZValue(10);
    m_scene->addItem(portLbl);

    QGraphicsRectItem *portBox = new QGraphicsRectItem(780, PANEL_Y + 87, 60, 20);
    portBox->setBrush(QBrush(C_WIN_LIGHT));
    portBox->setPen(QPen(Qt::darkGray, 1));
    portBox->setZValue(10);
    m_scene->addItem(portBox);

    QGraphicsSimpleTextItem *portValue = new QGraphicsSimpleTextItem("5025", nullptr);
    portValue->setFont(QFont("Arial", 9));
    portValue->setBrush(QBrush(Qt::black));
    portValue->setPos(795, PANEL_Y + 90);
    portValue->setZValue(11);
    m_scene->addItem(portValue);

    // Recipe
    QGraphicsSimpleTextItem *recipeLbl = new QGraphicsSimpleTextItem("Recipe:", nullptr);
    recipeLbl->setFont(QFont("Arial", 9));
    recipeLbl->setBrush(QBrush(Qt::black));
    recipeLbl->setPos(870, PANEL_Y + 63);
    recipeLbl->setZValue(10);
    m_scene->addItem(recipeLbl);

    QGraphicsSimpleTextItem *recipeValue = new QGraphicsSimpleTextItem("None", nullptr);
    recipeValue->setFont(QFont("Arial", 9));
    recipeValue->setBrush(QBrush(Qt::black));
    recipeValue->setPos(915, PANEL_Y + 63);
    recipeValue->setZValue(10);
    m_scene->addItem(recipeValue);

    // Step
    QGraphicsSimpleTextItem *stepLbl = new QGraphicsSimpleTextItem("Step:", nullptr);
    stepLbl->setFont(QFont("Arial", 9));
    stepLbl->setBrush(QBrush(Qt::black));
    stepLbl->setPos(870, PANEL_Y + 90);
    stepLbl->setZValue(10);
    m_scene->addItem(stepLbl);

    QGraphicsSimpleTextItem *stepValue = new QGraphicsSimpleTextItem("0/0", nullptr);
    stepValue->setFont(QFont("Arial", 9));
    stepValue->setBrush(QBrush(Qt::black));
    stepValue->setPos(905, PANEL_Y + 90);
    stepValue->setZValue(10);
    m_scene->addItem(stepValue);

    // Cycle
    QGraphicsSimpleTextItem *cycleLbl = new QGraphicsSimpleTextItem("Cycle:", nullptr);
    cycleLbl->setFont(QFont("Arial", 9));
    cycleLbl->setBrush(QBrush(Qt::black));
    cycleLbl->setPos(960, PANEL_Y + 90);
    cycleLbl->setZValue(10);
    m_scene->addItem(cycleLbl);

    QGraphicsSimpleTextItem *cycleValue = new QGraphicsSimpleTextItem("0/0", nullptr);
    cycleValue->setFont(QFont("Arial", 9));
    cycleValue->setBrush(QBrush(Qt::black));
    cycleValue->setPos(1000, PANEL_Y + 90);
    cycleValue->setZValue(10);
    m_scene->addItem(cycleValue);

    // Legend
    QGraphicsSimpleTextItem *legendTitle = new QGraphicsSimpleTextItem("Legend:", nullptr);
    legendTitle->setFont(QFont("Arial", 10, QFont::Bold));
    legendTitle->setBrush(QBrush(Qt::black));
    legendTitle->setPos(740, PANEL_Y + 115);
    legendTitle->setZValue(10);
    m_scene->addItem(legendTitle);

    LEDIndicator *vOpenLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN);
    vOpenLed->setPos(800, PANEL_Y + 112);
    vOpenLed->setZValue(10);
    m_scene->addItem(vOpenLed);
    QGraphicsSimpleTextItem *vOpenLbl = new QGraphicsSimpleTextItem("Valve Open", nullptr);
    vOpenLbl->setFont(QFont("Arial", 8));
    vOpenLbl->setBrush(QBrush(Qt::black));
    vOpenLbl->setPos(815, PANEL_Y + 115);
    vOpenLbl->setZValue(10);
    m_scene->addItem(vOpenLbl);

    LEDIndicator *vClosedLed = new LEDIndicator(true, C_LED_RED, C_LED_RED);
    vClosedLed->setPos(895, PANEL_Y + 112);
    vClosedLed->setZValue(10);
    m_scene->addItem(vClosedLed);
    QGraphicsSimpleTextItem *vClosedLbl = new QGraphicsSimpleTextItem("Valve Closed", nullptr);
    vClosedLbl->setFont(QFont("Arial", 8));
    vClosedLbl->setBrush(QBrush(Qt::black));
    vClosedLbl->setPos(910, PANEL_Y + 115);
    vClosedLbl->setZValue(10);
    m_scene->addItem(vClosedLbl);

    LEDIndicator *pumpRunLed = new LEDIndicator(true, C_LED_GREEN, C_LED_GREEN);
    pumpRunLed->setPos(990, PANEL_Y + 112);
    pumpRunLed->setZValue(10);
    m_scene->addItem(pumpRunLed);
    QGraphicsSimpleTextItem *pumpRunLbl = new QGraphicsSimpleTextItem("Pump Running", nullptr);
    pumpRunLbl->setFont(QFont("Arial", 8));
    pumpRunLbl->setBrush(QBrush(Qt::black));
    pumpRunLbl->setPos(1005, PANEL_Y + 115);
    pumpRunLbl->setZValue(10);
    m_scene->addItem(pumpRunLbl);
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
