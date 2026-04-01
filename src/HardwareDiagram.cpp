#include "HardwareDiagram.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QStyle>
#include <QLCDNumber>
#include <QSpacerItem>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsSimpleTextItem>
#include <QDebug>

// 颜色定义
static const QColor C_WIN_GRAY(192, 192, 192);
static const QColor C_WIN_DARK(128, 128, 128);
static const QColor C_WIN_LIGHT(224, 224, 224);
static const QColor C_WIN_MEDIUM(165, 165, 165);
static const QColor C_TITLE_BLUE(0, 0, 128);
static const QColor C_LED_GREEN(0, 200, 0);
static const QColor C_LED_GREEN_DARK(0, 100, 0);
static const QColor C_LED_RED(255, 0, 0);
static const QColor C_LED_RED_DARK(180, 0, 0);
static const QColor C_PIPE_BLACK(0, 0, 0);
static const QColor C_CHAMBER_OUTER(80, 80, 80);
static const QColor C_CHAMBER_MID(120, 120, 120);
static const QColor C_CHAMBER_INNER(180, 180, 180);
static const QColor C_WHITE(255, 255, 255);
static const QColor C_MENU_BG(212, 208, 200);

// ==================== ValveGraphicsItem - 阀门图元 ====================
class ValveGraphicsItem : public QGraphicsItemGroup
{
public:
    ValveGraphicsItem() : m_open(false)
    {
        setFlag(QGraphicsItem::ItemIsSelectable);

        // 阀门三角形
        QPainterPath path;
        path.moveTo(0, -8);
        path.lineTo(10, 0);
        path.lineTo(-10, 0);
        path.closeSubpath();

        QGraphicsPathItem *topTri = new QGraphicsPathItem(path, this);
        topTri->setBrush(QBrush(m_open ? C_LED_GREEN : C_LED_RED));
        topTri->setPen(QPen(Qt::black, 1));

        QGraphicsPathItem *bottomTri = new QGraphicsPathItem(path, this);
        bottomTri->setBrush(QBrush(m_open ? C_LED_GREEN : C_LED_RED));
        bottomTri->setPen(QPen(Qt::black, 1));
        bottomTri->setRotation(180);

        QGraphicsEllipseItem *center = new QGraphicsEllipseItem(-3, -3, 6, 6, this);
        center->setBrush(QBrush(Qt::darkGray));
        center->setPen(Qt::NoPen);
    }

    void setOpen(bool open) {
        m_open = open;
        // Update colors
        for (auto child : childItems()) {
            if (auto pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(child)) {
                pathItem->setBrush(QBrush(m_open ? C_LED_GREEN : C_LED_RED));
            }
        }
    }

    bool isOpen() const { return m_open; }

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        setOpen(!m_open);
        QGraphicsItemGroup::mousePressEvent(event);
    }

private:
    bool m_open;
};

// ==================== MimicWidget - 设备示意图 ====================
class MimicWidget : public QGraphicsView
{
public:
    MimicWidget(QWidget *parent = nullptr) : QGraphicsView(parent)
    {
        m_scene = new QGraphicsScene(this);
        setScene(m_scene);
        setRenderHint(QPainter::Antialiasing);
        setBackgroundBrush(QBrush(C_MENU_BG));
        setFrameShape(NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setFixedHeight(350);

        setupScene();
    }

    void setValveOpen(const QString &name, bool open) {
        if (m_valves.contains(name)) {
            m_valves[name]->setOpen(open);
        }
    }

private:
    void setupScene() {
        m_scene->clear();
        m_scene->setSceneRect(0, 0, 1100, 340);

        // ===== 气体 MFC (最左侧) =====
        struct GasMFC { const char* name; const char* gas; int y; };
        GasMFC mfcs[] = {
            {"CH1", "Ar", 20},
            {"CH2", "O2", 65},
            {"CH3", "N2", 110},
            {"CH4", "CF4", 155}
        };
        for (int i = 0; i < 4; i++) {
            QGraphicsRectItem *mfc = m_scene->addRect(10, mfcs[i].y, 55, 38);
            mfc->setBrush(QBrush(C_WIN_DARK));
            mfc->setPen(QPen(C_WIN_DARK, 1));

            QGraphicsSimpleTextItem *chLabel = new QGraphicsSimpleTextItem(mfcs[i].name);
            chLabel->setFont(QFont("Arial", 9, QFont::Bold));
            chLabel->setPos(18, mfcs[i].y + 3);
            m_scene->addItem(chLabel);

            QGraphicsSimpleTextItem *gasLabel = new QGraphicsSimpleTextItem(mfcs[i].gas);
            gasLabel->setFont(QFont("Arial", 8));
            gasLabel->setPos(22, mfcs[i].y + 16);
            m_scene->addItem(gasLabel);

            QGraphicsSimpleTextItem *flowVal = new QGraphicsSimpleTextItem("0.0");
            flowVal->setFont(QFont("Courier New", 8));
            flowVal->setPos(18, mfcs[i].y + 26);
            m_scene->addItem(flowVal);
        }

        // ===== Robot Arm 状态面板 (左侧,气体下方) =====
        QGraphicsRectItem *robotPanel = m_scene->addRect(10, 200, 150, 100);
        robotPanel->setBrush(QBrush(C_WIN_GRAY));
        robotPanel->setPen(QPen(C_WIN_DARK, 1));

        QGraphicsSimpleTextItem *robotTitle = new QGraphicsSimpleTextItem("Robot Arm");
        robotTitle->setFont(QFont("Arial", 9, QFont::Bold));
        robotTitle->setPos(15, 205);
        m_scene->addItem(robotTitle);

        // ARM HOME (绿色指示灯)
        addLedIndicator(15, 225, true, C_LED_GREEN);
        QGraphicsSimpleTextItem *armHome = new QGraphicsSimpleTextItem("ARM HOME");
        armHome->setFont(QFont("Arial", 8));
        armHome->setPos(30, 227);
        m_scene->addItem(armHome);

        // ARM EXTENDED (黑色指示灯)
        addLedIndicator(15, 243, false, C_WIN_DARK);
        QGraphicsSimpleTextItem *armExt = new QGraphicsSimpleTextItem("ARM EXTENDED");
        armExt->setFont(QFont("Arial", 8));
        armExt->setPos(30, 245);
        m_scene->addItem(armExt);

        // ARM FAULT (黑色指示灯)
        addLedIndicator(15, 261, false, C_WIN_DARK);
        QGraphicsSimpleTextItem *armFault = new QGraphicsSimpleTextItem("ARM FAULT");
        armFault->setFont(QFont("Arial", 8));
        armFault->setPos(30, 263);
        m_scene->addItem(armFault);

        // WAFER LIFT DOWN 标签
        QGraphicsSimpleTextItem *waferLift = new QGraphicsSimpleTextItem("WAFER LIFT DOWN");
        waferLift->setFont(QFont("Arial", 7));
        waferLift->setPos(15, 282);
        m_scene->addItem(waferLift);

        // 绘制 Load Lock (左侧中间)
        QGraphicsEllipseItem *llOuter = m_scene->addEllipse(180, 60, 80, 80);
        llOuter->setBrush(QBrush(C_CHAMBER_OUTER));
        llOuter->setPen(QPen(C_WIN_DARK, 2));

        QGraphicsEllipseItem *llInner = m_scene->addEllipse(190, 70, 60, 60);
        llInner->setBrush(QBrush(C_CHAMBER_INNER));
        llInner->setPen(Qt::NoPen);

        QGraphicsSimpleTextItem *loadLockLabel = new QGraphicsSimpleTextItem("Load Lock");
        loadLockLabel->setFont(QFont("Arial", 8));
        loadLockLabel->setPos(195, 92);
        m_scene->addItem(loadLockLabel);

        // 绘制主腔室 (中心)
        QGraphicsEllipseItem *chamberOuter = m_scene->addEllipse(480, 100, 140, 140);
        chamberOuter->setBrush(QBrush(C_CHAMBER_OUTER));
        chamberOuter->setPen(QPen(C_WIN_DARK, 2));

        QGraphicsEllipseItem *chamberRing2 = m_scene->addEllipse(495, 115, 110, 110);
        chamberRing2->setBrush(QBrush(C_CHAMBER_MID));
        chamberRing2->setPen(Qt::NoPen);

        QGraphicsEllipseItem *chamberRing3 = m_scene->addEllipse(510, 130, 80, 80);
        chamberRing3->setBrush(QBrush(C_CHAMBER_INNER));
        chamberRing3->setPen(Qt::NoPen);

        QGraphicsEllipseItem *chamberCenter = m_scene->addEllipse(530, 150, 40, 40);
        chamberCenter->setBrush(QBrush(QColor(60, 60, 60)));
        chamberCenter->setPen(Qt::NoPen);

        QGraphicsSimpleTextItem *chamberNum = new QGraphicsSimpleTextItem("1");
        chamberNum->setFont(QFont("Arial", 10, QFont::Bold));
        chamberNum->setPos(544, 164);
        m_scene->addItem(chamberNum);

        // Chuck 样品架
        QGraphicsRectItem *chuck = m_scene->addRect(530, 260, 40, 20);
        chuck->setBrush(QBrush(C_WIN_DARK));
        chuck->setPen(QPen(C_WIN_DARK, 1));
        QGraphicsSimpleTextItem *chuckLabel = new QGraphicsSimpleTextItem("Chuck");
        chuckLabel->setFont(QFont("Arial", 7));
        chuckLabel->setPos(533, 282);
        m_scene->addItem(chuckLabel);

        // ICP Source (上方)
        QGraphicsRectItem *icp = m_scene->addRect(520, 20, 60, 50);
        icp->setBrush(QBrush(C_WIN_DARK));
        icp->setPen(QPen(C_WIN_DARK, 1));
        QGraphicsSimpleTextItem *icpFreq = new QGraphicsSimpleTextItem("2 MHz");
        icpFreq->setFont(QFont("Arial", 10, QFont::Bold));
        icpFreq->setPos(530, 25);
        m_scene->addItem(icpFreq);
        QGraphicsSimpleTextItem *icpLabel = new QGraphicsSimpleTextItem("ICP");
        icpLabel->setFont(QFont("Arial", 8));
        icpLabel->setPos(535, 50);
        m_scene->addItem(icpLabel);

        // RF Generator (左侧)
        QGraphicsRectItem *rf = m_scene->addRect(330, 130, 60, 50);
        rf->setBrush(QBrush(C_WIN_DARK));
        rf->setPen(QPen(C_WIN_DARK, 1));
        QGraphicsSimpleTextItem *rfFreq = new QGraphicsSimpleTextItem("13.56");
        rfFreq->setFont(QFont("Arial", 10, QFont::Bold));
        rfFreq->setPos(340, 135);
        m_scene->addItem(rfFreq);
        QGraphicsSimpleTextItem *rfMhz = new QGraphicsSimpleTextItem("MHz");
        rfMhz->setFont(QFont("Arial", 7));
        rfMhz->setPos(347, 150);
        m_scene->addItem(rfMhz);
        QGraphicsSimpleTextItem *rfLabel = new QGraphicsSimpleTextItem("RF");
        rfLabel->setFont(QFont("Arial", 8));
        rfLabel->setPos(350, 165);
        m_scene->addItem(rfLabel);

        // 泵 (底部)
        struct Pump { const char* name; int x; };
        Pump pumps[] = {
            {"MP", 200},
            {"TP", 750}
        };
        for (int i = 0; i < 2; i++) {
            QGraphicsRectItem *pump = m_scene->addRect(pumps[i].x, 290, 80, 35);
            pump->setBrush(QBrush(C_WIN_DARK));
            pump->setPen(QPen(C_WIN_DARK, 1));
            QGraphicsSimpleTextItem *pLabel = new QGraphicsSimpleTextItem("P");
            pLabel->setFont(QFont("Arial", 14, QFont::Bold));
            pLabel->setPos(pumps[i].x + 30, 298);
            m_scene->addItem(pLabel);
            QGraphicsSimpleTextItem *pName = new QGraphicsSimpleTextItem(pumps[i].name);
            pName->setFont(QFont("Arial", 8));
            pName->setPos(pumps[i].x + 5, 318);
            m_scene->addItem(pName);
        }

        // 绘制管道
        QPen pipePen(C_PIPE_BLACK, 3);

        // Load Lock 到 Chamber
        m_scene->addLine(260, 100, 480, 100, pipePen);

        // Chamber 到 MP
        m_scene->addLine(530, 200, 530, 250, pipePen);
        m_scene->addLine(530, 290, 280, 290, pipePen);

        // Chamber 到 TP
        m_scene->addLine(560, 200, 560, 250, pipePen);
        m_scene->addLine(560, 290, 830, 290, pipePen);

        // RF 连接
        m_scene->addLine(390, 155, 480, 155, pipePen);

        // ICP 连接
        m_scene->addLine(550, 70, 550, 100, pipePen);

        // 气体管道 (从MFC到Chamber)
        m_scene->addLine(65, 38, 65, 130, pipePen);
        m_scene->addLine(65, 130, 480, 130, pipePen);

        // 创建阀门
        createValve("V_LL", 270, 85);
        createValve("V1", 530, 235);
        createValve("V2", 530, 275);
        createValve("V_GAS", 65, 115);

        // 压力计 (右侧)
        QGraphicsRectItem *gauge = m_scene->addRect(920, 130, 80, 70);
        gauge->setBrush(QBrush(C_WIN_GRAY));
        gauge->setPen(QPen(C_WIN_DARK, 1));

        QGraphicsRectItem *gaugeDisplay = m_scene->addRect(925, 135, 70, 25);
        gaugeDisplay->setBrush(QBrush(Qt::black));
        gaugeDisplay->setPen(Qt::NoPen);

        QGraphicsTextItem *pressVal = new QGraphicsTextItem("0.00");
        pressVal->setFont(QFont("Courier New", 12));
        pressVal->setPos(938, 138);
        pressVal->setDefaultTextColor(C_LED_GREEN);
        m_scene->addItem(pressVal);

        QGraphicsSimpleTextItem *mbarLabel = new QGraphicsSimpleTextItem("mbar");
        mbarLabel->setFont(QFont("Arial", 8));
        mbarLabel->setPos(940, 158);
        m_scene->addItem(mbarLabel);

        QGraphicsSimpleTextItem *piraniLabel = new QGraphicsSimpleTextItem("Pirani");
        piraniLabel->setFont(QFont("Arial", 8));
        piraniLabel->setPos(938, 180);
        m_scene->addItem(piraniLabel);

        // Turbo Pump 状态 (右上角)
        QGraphicsRectItem *turboPanel = m_scene->addRect(920, 10, 80, 65);
        turboPanel->setBrush(QBrush(C_WIN_GRAY));
        turboPanel->setPen(QPen(C_WIN_DARK, 1));

        QGraphicsSimpleTextItem *turboLabel = new QGraphicsSimpleTextItem("Turbo");
        turboLabel->setFont(QFont("Arial", 8));
        turboLabel->setPos(940, 15);
        m_scene->addItem(turboLabel);

        addLedIndicator(925, 35, true, C_LED_GREEN);

        QGraphicsSimpleTextItem *atSpeed = new QGraphicsSimpleTextItem("at speed");
        atSpeed->setFont(QFont("Arial", 8));
        atSpeed->setPos(940, 38);
        m_scene->addItem(atSpeed);

        QGraphicsSimpleTextItem *n2Label = new QGraphicsSimpleTextItem("N2:");
        n2Label->setFont(QFont("Arial", 8));
        n2Label->setPos(925, 55);
        m_scene->addItem(n2Label);

        QGraphicsSimpleTextItem *n2Val = new QGraphicsSimpleTextItem("0.0 sccm");
        n2Val->setFont(QFont("Courier New", 8));
        n2Val->setPos(955, 55);
        m_scene->addItem(n2Val);

        // "Robot arm standing by" 横幅
        QGraphicsRectItem *banner = m_scene->addRect(180, 10, 300, 25);
        banner->setBrush(QBrush(C_TITLE_BLUE));
        banner->setPen(Qt::NoPen);
        QGraphicsTextItem *bannerText = new QGraphicsTextItem("Robot arm standing by");
        bannerText->setFont(QFont("Arial", 10));
        bannerText->setPos(190, 15);
        bannerText->setDefaultTextColor(Qt::white);
        m_scene->addItem(bannerText);
    }

    void addLedIndicator(int x, int y, bool on, QColor color) {
        QGraphicsEllipseItem *led = m_scene->addEllipse(x, y, 10, 10);
        led->setBrush(QBrush(on ? color : C_WIN_DARK));
        led->setPen(QPen(Qt::darkGray, 1));
    }

    void createValve(const QString &name, int x, int y) {
        ValveGraphicsItem *valve = new ValveGraphicsItem();
        valve->setPos(x, y);
        m_scene->addItem(valve);
        m_valves[name] = valve;
    }

    QGraphicsScene *m_scene;
    QMap<QString, ValveGraphicsItem*> m_valves;
};

// ==================== StatusPanel - 状态面板 ====================
class StatusPanel : public QFrame
{
public:
    StatusPanel(const QString &title, QWidget *parent = nullptr) : QFrame(parent)
    {
        setFrameStyle(QFrame::Panel | QFrame::Sunken);
        setStyleSheet(QString(
            "QFrame { background-color: %1; border: 1px solid %2; padding: 5px; } "
            "QLabel { font-family: Arial; font-size: 9pt; } "
            "QPushButton { background-color: %1; border: 2px outset %3; padding: 5px 10px; font-family: Arial; font-size: 9pt; min-width: 70px; } "
            "QPushButton:pressed { border-style: inset; background-color: %4; }"
        ).arg(C_WIN_GRAY.name()).arg(C_WIN_DARK.name()).arg(C_WIN_LIGHT.name()).arg(C_WIN_MEDIUM.name()));

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(8, 8, 8, 8);
        mainLayout->setSpacing(5);

        // 标题
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setStyleSheet("font-weight: bold; font-size: 10pt;");
        mainLayout->addWidget(titleLabel);

        // 状态显示
        m_statusLabel = new QLabel("Stopped Pumping/Venting");
        m_statusLabel->setStyleSheet("color: " + C_LED_GREEN.name() + "; font-weight: bold;");
        mainLayout->addWidget(m_statusLabel);

        // 参数网格
        QGridLayout *grid = new QGridLayout();
        grid->setSpacing(5);

        // Lid
        grid->addWidget(new QLabel("Lid:"), 0, 0);
        m_lidValue = new QLabel("CLOSED");
        m_lidValue->setStyleSheet("background-color: " + C_WIN_LIGHT.name() + "; padding: 2px 5px; border: 1px solid " + C_WIN_DARK.name() + ";");
        grid->addWidget(m_lidValue, 0, 1);

        // Pirani
        grid->addWidget(new QLabel("Pirani:"), 1, 0);
        m_piraniValue = new QLabel("6.19e-05 Torr");
        m_piraniValue->setStyleSheet("background-color: black; color: " + C_LED_GREEN.name() + "; padding: 2px 5px; font-family: 'Courier New';");
        grid->addWidget(m_piraniValue, 1, 1);

        // Interlock
        grid->addWidget(new QLabel("Interlock:"), 2, 0);
        m_interlockValue = new QLabel("OK");
        m_interlockValue->setStyleSheet("color: " + C_LED_GREEN.name() + "; font-weight: bold;");
        grid->addWidget(m_interlockValue, 2, 1);

        // Vent Time
        grid->addWidget(new QLabel("Vent:"), 3, 0);
        m_ventValue = new QLabel("0 s");
        m_ventValue->setStyleSheet("background-color: " + C_WIN_LIGHT.name() + "; padding: 2px 5px;");
        grid->addWidget(m_ventValue, 3, 1);

        mainLayout->addLayout(grid);

        // 按钮行
        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(5);

        m_evacuateBtn = new QPushButton("EVACUATE");
        m_stopBtn = new QPushButton("STOP");
        m_ventBtn = new QPushButton("VENT");

        btnLayout->addWidget(m_evacuateBtn);
        btnLayout->addWidget(m_stopBtn);
        btnLayout->addWidget(m_ventBtn);

        mainLayout->addLayout(btnLayout);
    }

    void setStatus(const QString &status) { m_statusLabel->setText(status); }
    void setLid(const QString &lid) { m_lidValue->setText(lid); }
    void setPirani(const QString &pirani) { m_piraniValue->setText(pirani); }
    void setInterlock(const QString &interlock) { m_interlockValue->setText(interlock); }
    void setVent(const QString &vent) { m_ventValue->setText(vent); }

    QPushButton *m_evacuateBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_ventBtn;

private:
    QLabel *m_statusLabel;
    QLabel *m_lidValue;
    QLabel *m_piraniValue;
    QLabel *m_interlockValue;
    QLabel *m_ventValue;
};

// ==================== HardwareDiagram 主类 ====================

HardwareDiagram::HardwareDiagram(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

HardwareDiagram::~HardwareDiagram()
{
}

void HardwareDiagram::setupUi()
{
    // 设置窗口样式 - Windows Classic 风格
    setStyleSheet(QString(
        "QWidget { background-color: %1; } "
        "QPushButton { background-color: %1; border: 2px outset %2; padding: 5px 15px; font-family: Arial; font-size: 9pt; } "
        "QPushButton:pressed { border-style: inset; background-color: %3; }"
    ).arg(C_WIN_GRAY.name()).arg(C_WIN_LIGHT.name()).arg(C_WIN_MEDIUM.name()));

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // ===== 顶部导航栏 =====
    QFrame *topBar = new QFrame();
    topBar->setStyleSheet(QString("background-color: %1; border: 1px solid %2;").arg(C_WIN_GRAY.name()).arg(C_WIN_DARK.name()));
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    // 页面标题
    QLabel *pageTitle = new QLabel("Pump control page");
    pageTitle->setStyleSheet("font-weight: bold; font-size: 11pt;");
    topLayout->addWidget(pageTitle);

    topLayout->addSpacing(50);

    // PUMP CONTROL 标签
    QLabel *pumpLabel = new QLabel("PUMP CONTROL");
    pumpLabel->setStyleSheet("font-weight: bold;");
    topLayout->addWidget(pumpLabel);

    topLayout->addStretch();

    // LOG 按钮
    QPushButton *logBtn = new QPushButton("LOG");
    logBtn->setFixedSize(40, 30);
    topLayout->addWidget(logBtn);

    // STOP ALL AUTO PROCESSES 按钮
    QPushButton *stopAllBtn = new QPushButton("STOP ALL AUTO PROCESSES");
    stopAllBtn->setStyleSheet(QString("background-color: %1; color: white; font-weight: bold;").arg(C_LED_GREEN.name()));
    stopAllBtn->setFixedHeight(30);
    topLayout->addWidget(stopAllBtn);

    // STOP 按钮
    QPushButton *stopBtn = new QPushButton("STOP");
    stopBtn->setStyleSheet(QString("background-color: %1; color: white; font-weight: bold;").arg(C_LED_RED.name()));
    stopBtn->setFixedSize(60, 30);
    topLayout->addWidget(stopBtn);

    mainLayout->addWidget(topBar);

    // ===== Mimic 示意图 =====
    m_mimicWidget = new MimicWidget();
    mainLayout->addWidget(m_mimicWidget);

    // ===== 底部面板 =====
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(10);

    // Load-Lock Area
    m_loadLockPanel = new StatusPanel("Load-Lock Area");
    connect(m_loadLockPanel->m_evacuateBtn, &QPushButton::clicked, this, &HardwareDiagram::onEvacuateClicked);
    connect(m_loadLockPanel->m_stopBtn, &QPushButton::clicked, this, &HardwareDiagram::onStopClicked);
    connect(m_loadLockPanel->m_ventBtn, &QPushButton::clicked, this, &HardwareDiagram::onVentClicked);
    bottomLayout->addWidget(m_loadLockPanel, 1);

    // Process Chamber Area
    m_processChamberPanel = new StatusPanel("Process Chamber Area");
    m_processChamberPanel->setInterlock("OK");
    connect(m_processChamberPanel->m_evacuateBtn, &QPushButton::clicked, this, &HardwareDiagram::onEvacuateClicked);
    connect(m_processChamberPanel->m_stopBtn, &QPushButton::clicked, this, &HardwareDiagram::onStopClicked);
    connect(m_processChamberPanel->m_ventBtn, &QPushButton::clicked, this, &HardwareDiagram::onVentClicked);
    bottomLayout->addWidget(m_processChamberPanel, 1);

    // System Status
    m_systemStatusPanel = new StatusPanel("System Status");
    m_systemStatusPanel->setStatus("Disconnected");
    m_systemStatusPanel->m_evacuateBtn->setVisible(false);
    m_systemStatusPanel->m_stopBtn->setVisible(false);
    m_systemStatusPanel->m_ventBtn->setVisible(false);

    // 在 System Status 中添加连接信息
    QVBoxLayout *sysLayout = qobject_cast<QVBoxLayout*>(m_systemStatusPanel->layout());
    if (sysLayout) {
        QGridLayout *infoGrid = new QGridLayout();
        infoGrid->addWidget(new QLabel("Host:"), 0, 0);
        QLabel *hostVal = new QLabel("192.168.1.100");
        hostVal->setStyleSheet("background-color: " + C_WIN_LIGHT.name() + "; padding: 2px 5px;");
        infoGrid->addWidget(hostVal, 0, 1);

        infoGrid->addWidget(new QLabel("Port:"), 1, 0);
        QLabel *portVal = new QLabel("5025");
        portVal->setStyleSheet("background-color: " + C_WIN_LIGHT.name() + "; padding: 2px 5px;");
        infoGrid->addWidget(portVal, 1, 1);

        infoGrid->addWidget(new QLabel("Recipe:"), 2, 0);
        QLabel *recipeVal = new QLabel("None");
        infoGrid->addWidget(recipeVal, 2, 1);

        infoGrid->addWidget(new QLabel("Step:"), 3, 0);
        QLabel *stepVal = new QLabel("0/0");
        infoGrid->addWidget(stepVal, 3, 1);

        sysLayout->insertLayout(2, infoGrid);
    }
    bottomLayout->addWidget(m_systemStatusPanel, 1);

    mainLayout->addLayout(bottomLayout);
}

void HardwareDiagram::updateComponentState(const QString &name, ComponentState state)
{
    m_componentStates[name] = state;
}

void HardwareDiagram::updateComponentValue(const QString &name, const QString &value)
{
    if (name == "Pressure") {
        m_loadLockPanel->setPirani(value);
        m_processChamberPanel->setPirani(value);
    }
}

void HardwareDiagram::setValveOpen(const QString &name, bool open)
{
    m_mimicWidget->setValveOpen(name, open);
}

void HardwareDiagram::setPipeFlow(const QString &name, bool flowing)
{
    // 实现管道流动状态更新
}

ComponentState HardwareDiagram::getComponentState(const QString &name) const
{
    return m_componentStates.value(name, ComponentState::Unknown);
}

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