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
    // 创建3D渐变画刷
    QLinearGradient createMetalGradient(bool vertical = true) {
        QLinearGradient grad;
        if (vertical) {
            grad.setStart(0, 0);
            grad.setFinalStop(0, 1);
            grad.setColorAt(0, QColor(180, 180, 180));
            grad.setColorAt(0.3, QColor(140, 140, 140));
            grad.setColorAt(0.7, QColor(100, 100, 100));
            grad.setColorAt(1, QColor(80, 80, 80));
        } else {
            grad.setStart(0, 0);
            grad.setFinalStop(1, 0);
            grad.setColorAt(0, QColor(180, 180, 180));
            grad.setColorAt(0.3, QColor(140, 140, 140));
            grad.setColorAt(0.7, QColor(100, 100, 100));
            grad.setColorAt(1, QColor(80, 80, 80));
        }
        return grad;
    }

    QLinearGradient createChamberGradient() {
        QLinearGradient grad;
        grad.setStart(0, 0);
        grad.setFinalStop(1, 1);
        grad.setColorAt(0, QColor(100, 100, 110));
        grad.setColorAt(0.4, QColor(60, 60, 70));
        grad.setColorAt(0.6, QColor(50, 50, 60));
        grad.setColorAt(1, QColor(30, 30, 40));
        return grad;
    }

    void drawPipe(int x1, int y1, int x2, int y2, int width = 6) {
        // 管道主体 - 金属灰
        QPen pipePen(QColor(120, 120, 130), width);
        m_scene->addLine(x1, y1, x2, y2, pipePen);

        // 管道高光
        QPen highlightPen(QColor(180, 180, 190), 1);
        if (x1 == x2) { // 垂直管道
            m_scene->addLine(x1 - width/2 + 1, y1, x2 - width/2 + 1, y2, highlightPen);
        } else { // 水平管道
            m_scene->addLine(x1, y1 - width/2 + 1, x2, y2 - width/2 + 1, highlightPen);
        }
    }

    void drawValveBody(int x, int y, int width, int height) {
        // 阀门金属边框
        QLinearGradient grad = createMetalGradient();
        QGraphicsRectItem *valveBody = m_scene->addRect(x, y, width, height);
        valveBody->setBrush(QBrush(grad));
        valveBody->setPen(QPen(QColor(60, 60, 60), 1));

        // 阀门手柄
        QGraphicsRectItem *handle = m_scene->addRect(x + width/2 - 3, y - 5, 6, 8);
        handle->setBrush(QBrush(QColor(200, 80, 80)));
        handle->setPen(QPen(Qt::darkRed, 1));

        // 阀门中心指示灯
        QGraphicsEllipseItem *indicator = m_scene->addEllipse(x + width/2 - 3, y + height/2 - 3, 6, 6);
        indicator->setBrush(QBrush(C_LED_GREEN));
        indicator->setPen(QPen(Qt::darkGreen, 1));
    }

    void setupScene() {
        m_scene->clear();
        m_scene->setSceneRect(0, 0, 1100, 340);

        // ===== 气体 MFC 面板 (左侧) =====
        // MFC 机箱背景
        QGraphicsRectItem *mfcPanel = m_scene->addRect(5, 10, 70, 180);
        mfcPanel->setBrush(QBrush(QColor(60, 60, 65)));
        mfcPanel->setPen(QPen(QColor(40, 40, 45), 2));

        struct GasMFC { const char* name; const char* gas; int y; };
        GasMFC mfcs[] = {
            {"CH1", "Ar", 20},
            {"CH2", "O2", 65},
            {"CH3", "N2", 110},
            {"CH4", "CF4", 155}
        };
        for (int i = 0; i < 4; i++) {
            // MFC 主体 - 拟物金属盒
            QLinearGradient mfcGrad;
            mfcGrad.setStart(0, 0);
            mfcGrad.setFinalStop(0, 1);
            mfcGrad.setColorAt(0, QColor(160, 160, 165));
            mfcGrad.setColorAt(0.5, QColor(120, 120, 125));
            mfcGrad.setColorAt(1, QColor(90, 90, 95));

            QGraphicsRectItem *mfcBox = m_scene->addRect(10, mfcs[i].y, 60, 40);
            mfcBox->setBrush(QBrush(mfcGrad));
            mfcBox->setPen(QPen(QColor(50, 50, 55), 1));

            // MFC 显示屏
            QGraphicsRectItem *mfcDisplay = m_scene->addRect(13, mfcs[i].y + 3, 35, 15);
            mfcDisplay->setBrush(QBrush(Qt::black));
            mfcDisplay->setPen(Qt::NoPen);

            QGraphicsTextItem *flowVal = new QGraphicsTextItem("0.0");
            flowVal->setFont(QFont("Courier New", 10));
            flowVal->setPos(15, mfcs[i].y + 4);
            flowVal->setDefaultTextColor(C_LED_GREEN);
            m_scene->addItem(flowVal);

            // 通道标签
            QGraphicsSimpleTextItem *chLabel = new QGraphicsSimpleTextItem(mfcs[i].name);
            chLabel->setFont(QFont("Arial", 8, QFont::Bold));
            chLabel->setPos(50, mfcs[i].y + 5);
            m_scene->addItem(chLabel);

            // 气体标签
            QGraphicsSimpleTextItem *gasLabel = new QGraphicsSimpleTextItem(mfcs[i].gas);
            gasLabel->setFont(QFont("Arial", 9));
            gasLabel->setPos(20, mfcs[i].y + 22);
            m_scene->addItem(gasLabel);

            // MFC 进/出口指示
            QGraphicsEllipseItem *inlet = m_scene->addEllipse(5, mfcs[i].y + 15, 5, 10);
            inlet->setBrush(QBrush(QColor(80, 80, 80)));
            inlet->setPen(QPen(Qt::darkGray, 1));

            QGraphicsRectItem *outlet = m_scene->addRect(70, mfcs[i].y + 15, 5, 10);
            outlet->setBrush(QBrush(QColor(80, 80, 80)));
            outlet->setPen(QPen(Qt::darkGray, 1));
        }

        // MFC 面板标签
        QGraphicsSimpleTextItem *mfcTitle = new QGraphicsSimpleTextItem("MFC CONTROL");
        mfcTitle->setFont(QFont("Arial", 8, QFont::Bold));
        mfcTitle->setPos(15, 2);
        m_scene->addItem(mfcTitle);

        // ===== Robot Arm 状态面板 (左下方) =====
        QGraphicsRectItem *robotPanel = m_scene->addRect(5, 195, 150, 100);
        robotPanel->setBrush(QBrush(QColor(70, 70, 75)));
        robotPanel->setPen(QPen(QColor(50, 50, 55), 2));

        QGraphicsSimpleTextItem *robotTitle = new QGraphicsSimpleTextItem("ROBOT ARM");
        robotTitle->setFont(QFont("Arial", 9, QFont::Bold));
        robotTitle->setPos(10, 200);
        m_scene->addItem(robotTitle);

        // 状态指示灯
        addLedIndicator(10, 220, true, C_LED_GREEN);
        QGraphicsSimpleTextItem *armHome = new QGraphicsSimpleTextItem("HOME");
        armHome->setFont(QFont("Arial", 8));
        armHome->setPos(25, 222);
        m_scene->addItem(armHome);

        addLedIndicator(10, 238, false, C_WIN_DARK);
        QGraphicsSimpleTextItem *armExt = new QGraphicsSimpleTextItem("EXTENDED");
        armExt->setFont(QFont("Arial", 8));
        armExt->setPos(25, 240);
        m_scene->addItem(armExt);

        addLedIndicator(10, 256, false, C_LED_RED);
        QGraphicsSimpleTextItem *armFault = new QGraphicsSimpleTextItem("FAULT");
        armFault->setFont(QFont("Arial", 8));
        armFault->setPos(25, 258);
        m_scene->addItem(armFault);

        // WAFER LIFT 指示
        QGraphicsSimpleTextItem *waferLift = new QGraphicsSimpleTextItem("WAFER LIFT");
        waferLift->setFont(QFont("Arial", 7));
        waferLift->setPos(90, 222);
        m_scene->addItem(waferLift);

        addLedIndicator(90, 235, false, C_LED_GREEN);
        addLedIndicator(90, 250, true, C_LED_GREEN);

        QGraphicsSimpleTextItem *upLabel = new QGraphicsSimpleTextItem("UP");
        upLabel->setFont(QFont("Arial", 6));
        upLabel->setPos(105, 237);
        m_scene->addItem(upLabel);

        QGraphicsSimpleTextItem *downLabel = new QGraphicsSimpleTextItem("DOWN");
        downLabel->setFont(QFont("Arial", 6));
        downLabel->setPos(103, 252);
        m_scene->addItem(downLabel);

        // ===== Load Lock 腔体 (拟物圆形腔室) =====
        // Load Lock 外圈金属环
        QLinearGradient llOuterGrad;
        llOuterGrad.setStart(0, 0);
        llOuterGrad.setFinalStop(1, 1);
        llOuterGrad.setColorAt(0, QColor(140, 140, 150));
        llOuterGrad.setColorAt(0.5, QColor(100, 100, 110));
        llOuterGrad.setColorAt(1, QColor(70, 70, 80));

        QGraphicsEllipseItem *llOuter = m_scene->addEllipse(160, 50, 90, 90);
        llOuter->setBrush(QBrush(llOuterGrad));
        llOuter->setPen(QPen(QColor(50, 50, 60), 3));

        // Load Lock 法兰
        QGraphicsEllipseItem *llFlange = m_scene->addEllipse(168, 58, 74, 74);
        llFlange->setBrush(QBrush(QColor(80, 80, 85)));
        llFlange->setPen(QPen(Qt::darkGray, 2));

        // Load Lock 观察窗
        QGraphicsEllipseItem *llWindow = m_scene->addEllipse(180, 70, 50, 50);
        llWindow->setBrush(QBrush(QColor(30, 35, 40)));
        llWindow->setPen(QPen(QColor(60, 60, 70), 1));

        // 观察窗反光
        QGraphicsEllipseItem *llReflection = m_scene->addEllipse(185, 73, 15, 15);
        llReflection->setBrush(QBrush(QColor(100, 100, 110, 100)));
        llReflection->setPen(Qt::NoPen);

        QGraphicsSimpleTextItem *loadLockLabel = new QGraphicsSimpleTextItem("LOAD LOCK");
        loadLockLabel->setFont(QFont("Arial", 8, QFont::Bold));
        loadLockLabel->setPos(175, 92);
        m_scene->addItem(loadLockLabel);

        // Load Lock 门闩指示
        QGraphicsRectItem *llLatch = m_scene->addRect(200, 135, 20, 8);
        llLatch->setBrush(QBrush(QColor(150, 150, 150)));
        llLatch->setPen(QPen(Qt::darkGray, 1));

        // ===== 主腔室 (中心 - 拟物真空腔室) =====
        // 腔室外层金属框架
        QLinearGradient chamberFrameGrad;
        chamberFrameGrad.setStart(0, 0);
        chamberFrameGrad.setFinalStop(1, 1);
        chamberFrameGrad.setColorAt(0, QColor(130, 130, 140));
        chamberFrameGrad.setColorAt(0.3, QColor(110, 110, 120));
        chamberFrameGrad.setColorAt(0.7, QColor(80, 80, 90));
        chamberFrameGrad.setColorAt(1, QColor(60, 60, 70));

        QGraphicsEllipseItem *chamberFrame = m_scene->addEllipse(440, 70, 200, 200);
        chamberFrame->setBrush(QBrush(chamberFrameGrad));
        chamberFrame->setPen(QPen(QColor(40, 40, 50), 4));

        // 腔室主体
        QGraphicsEllipseItem *chamberBody = m_scene->addEllipse(455, 85, 170, 170);
        chamberBody->setBrush(QBrush(createChamberGradient()));
        chamberBody->setPen(QPen(QColor(50, 50, 60), 2));

        // 内部散热片/绝缘层
        for (int i = 0; i < 8; i++) {
            QGraphicsRectItem *fin = m_scene->addRect(475 + i*18, 100, 8, 140);
            fin->setBrush(QBrush(QColor(45, 45, 55, 150)));
            fin->setPen(Qt::NoPen);
        }

        // 腔室观察窗
        QGraphicsEllipseItem *chamberWindow = m_scene->addEllipse(500, 130, 80, 80);
        chamberWindow->setBrush(QBrush(QColor(20, 25, 30)));
        chamberWindow->setPen(QPen(QColor(70, 70, 80), 2));

        // 等离子发光效果
        QGraphicsEllipseItem *plasma = m_scene->addEllipse(520, 150, 40, 40);
        plasma->setBrush(QBrush(QColor(100, 150, 255, 150)));
        plasma->setPen(Qt::NoPen);

        // 观察窗玻璃反光
        QGraphicsEllipseItem *chamReflection = m_scene->addEllipse(510, 138, 20, 20);
        chamReflection->setBrush(QBrush(QColor(150, 150, 160, 80)));
        chamReflection->setPen(Qt::NoPen);

        // 腔室编号
        QGraphicsTextItem *chamberNum = new QGraphicsTextItem("1");
        chamberNum->setFont(QFont("Arial", 16, QFont::Bold));
        chamberNum->setPos(533, 163);
        chamberNum->setDefaultTextColor(QColor(200, 200, 210));
        m_scene->addItem(chamberNum);

        // ===== Chuck (样品台) =====
        QLinearGradient chuckGrad;
        chuckGrad.setStart(0, 0);
        chuckGrad.setFinalStop(0, 1);
        chuckGrad.setColorAt(0, QColor(180, 180, 190));
        chuckGrad.setColorAt(0.5, QColor(140, 140, 150));
        chuckGrad.setColorAt(1, QColor(100, 100, 110));

        QGraphicsRectItem *chuck = m_scene->addRect(510, 270, 60, 25);
        chuck->setBrush(QBrush(chuckGrad));
        chuck->setPen(QPen(QColor(60, 60, 70), 2));

        QGraphicsSimpleTextItem *chuckLabel = new QGraphicsSimpleTextItem("CHUCK");
        chuckLabel->setFont(QFont("Arial", 7, QFont::Bold));
        chuckLabel->setPos(525, 278);
        m_scene->addItem(chuckLabel);

        // Chuck 电极指示
        QGraphicsRectItem *chuckElectrode = m_scene->addRect(525, 293, 30, 5);
        chuckElectrode->setBrush(QBrush(QColor(80, 80, 100)));
        chuckElectrode->setPen(Qt::NoPen);

        // ===== ICP Source (上方 - 感应线圈) =====
        QLinearGradient icpGrad;
        icpGrad.setStart(0, 0);
        icpGrad.setFinalStop(0, 1);
        icpGrad.setColorAt(0, QColor(160, 80, 80));
        icpGrad.setColorAt(0.5, QColor(120, 60, 60));
        icpGrad.setColorAt(1, QColor(90, 40, 40));

        QGraphicsRectItem *icpBase = m_scene->addRect(500, 15, 80, 55);
        icpBase->setBrush(QBrush(icpGrad));
        icpBase->setPen(QPen(QColor(60, 30, 30), 2));

        // ICP 线圈
        for (int i = 0; i < 3; i++) {
            QGraphicsEllipseItem *coil = m_scene->addEllipse(510 + i*20, 25, 30, 35);
            coil->setBrush(Qt::NoBrush);
            coil->setPen(QPen(QColor(200, 150, 100), 3));
        }

        QGraphicsSimpleTextItem *icpLabel = new QGraphicsSimpleTextItem("ICP");
        icpLabel->setFont(QFont("Arial", 12, QFont::Bold));
        icpLabel->setPos(540, 48);
        m_scene->addItem(icpLabel);

        QGraphicsSimpleTextItem *icpFreq = new QGraphicsSimpleTextItem("2 MHz");
        icpFreq->setFont(QFont("Arial", 8));
        icpFreq->setPos(580, 22);
        m_scene->addItem(icpFreq);

        // ===== RF Generator (左侧) =====
        QLinearGradient rfGrad;
        rfGrad.setStart(0, 0);
        rfGrad.setFinalStop(1, 1);
        rfGrad.setColorAt(0, QColor(100, 100, 120));
        rfGrad.setColorAt(0.5, QColor(70, 70, 90));
        rfGrad.setColorAt(1, QColor(50, 50, 70));

        QGraphicsRectItem *rfBox = m_scene->addRect(300, 100, 100, 90);
        rfBox->setBrush(QBrush(rfGrad));
        rfBox->setPen(QPen(QColor(40, 40, 60), 2));

        // RF 显示屏
        QGraphicsRectItem *rfDisplay = m_scene->addRect(305, 105, 60, 25);
        rfDisplay->setBrush(QBrush(Qt::black));
        rfDisplay->setPen(Qt::NoPen);

        QGraphicsTextItem *rfPower = new QGraphicsTextItem("0 W");
        rfPower->setFont(QFont("Courier New", 12));
        rfPower->setPos(315, 108);
        rfPower->setDefaultTextColor(C_LED_GREEN);
        m_scene->addItem(rfPower);

        QGraphicsSimpleTextItem *rfFreq = new QGraphicsSimpleTextItem("13.56 MHz");
        rfFreq->setFont(QFont("Arial", 9));
        rfFreq->setPos(305, 135);
        m_scene->addItem(rfFreq);

        QGraphicsSimpleTextItem *rfLabel = new QGraphicsSimpleTextItem("RF");
        rfLabel->setFont(QFont("Arial", 10, QFont::Bold));
        rfLabel->setPos(375, 130);
        m_scene->addItem(rfLabel);

        // RF 指示灯
        addLedIndicator(380, 150, false, C_LED_GREEN);
        addLedIndicator(380, 165, true, C_LED_RED);

        // ===== 机械泵 MP (底部左侧) =====
        QLinearGradient mpGrad;
        mpGrad.setStart(0, 0);
        mpGrad.setFinalStop(1, 1);
        mpGrad.setColorAt(0, QColor(130, 130, 140));
        mpGrad.setColorAt(0.5, QColor(100, 100, 110));
        mpGrad.setColorAt(1, QColor(70, 70, 80));

        QGraphicsRectItem *mpBody = m_scene->addRect(170, 270, 120, 55);
        mpBody->setBrush(QBrush(mpGrad));
        mpBody->setPen(QPen(QColor(50, 50, 60), 2));

        // 泵风扇罩
        QGraphicsEllipseItem *mpFan = m_scene->addEllipse(180, 278, 35, 35);
        mpFan->setBrush(QBrush(QColor(60, 60, 65)));
        mpFan->setPen(QPen(QColor(80, 80, 85), 2));

        // 风扇叶片
        for (int i = 0; i < 6; i++) {
            QGraphicsRectItem *blade = m_scene->addRect(195, 280, 4, 30);
            blade->setBrush(QBrush(QColor(80, 80, 85)));
            blade->setPen(Qt::NoPen);
            blade->setRotation(60 * i);
            blade->setTransformOriginPoint(197, 295);
        }

        // 泵标签
        QGraphicsSimpleTextItem *mpLabel = new QGraphicsSimpleTextItem("MECHANICAL PUMP");
        mpLabel->setFont(QFont("Arial", 7, QFont::Bold));
        mpLabel->setPos(220, 280);
        m_scene->addItem(mpLabel);

        QGraphicsSimpleTextItem *mpType = new QGraphicsSimpleTextItem("MP");
        mpType->setFont(QFont("Arial", 14, QFont::Bold));
        mpType->setPos(220, 300);
        m_scene->addItem(mpType);

        // ===== 涡轮泵 TP (底部右侧) =====
        QLinearGradient tpGrad;
        tpGrad.setStart(0, 0);
        tpGrad.setFinalStop(1, 1);
        tpGrad.setColorAt(0, QColor(120, 120, 130));
        tpGrad.setColorAt(0.5, QColor(90, 90, 100));
        tpGrad.setColorAt(1, QColor(60, 60, 70));

        QGraphicsRectItem *tpBody = m_scene->addRect(720, 265, 120, 65);
        tpBody->setBrush(QBrush(tpGrad));
        tpBody->setPen(QPen(QColor(40, 40, 50), 2));

        // 涡轮泵入口
        QGraphicsEllipseItem *tpInlet = m_scene->addEllipse(730, 255, 40, 25);
        tpInlet->setBrush(QBrush(QColor(50, 50, 60)));
        tpInlet->setPen(QPen(QColor(60, 60, 70), 2));

        // 涡轮叶片
        QGraphicsEllipseItem *tpFan = m_scene->addEllipse(750, 280, 50, 45);
        tpFan->setBrush(QBrush(QColor(70, 70, 80)));
        tpFan->setPen(QPen(QColor(90, 90, 100), 2));

        for (int i = 0; i < 8; i++) {
            QGraphicsRectItem *tBlade = m_scene->addRect(773, 285, 3, 35);
            tBlade->setBrush(QBrush(QColor(100, 100, 110)));
            tBlade->setPen(Qt::NoPen);
            tBlade->setRotation(45 * i);
            tBlade->setTransformOriginPoint(774, 302);
        }

        QGraphicsSimpleTextItem *tpLabel = new QGraphicsSimpleTextItem("TURBO PUMP");
        tpLabel->setFont(QFont("Arial", 8, QFont::Bold));
        tpLabel->setPos(810, 285);
        m_scene->addItem(tpLabel);

        QGraphicsSimpleTextItem *tpType = new QGraphicsSimpleTextItem("TP");
        tpType->setFont(QFont("Arial", 14, QFont::Bold));
        tpType->setPos(810, 305);
        m_scene->addItem(tpType);

        // ===== 绘制管道 (拟物风格) =====
        // Load Lock 到 Chamber 管道
        drawPipe(250, 95, 440, 95, 8);

        // Chamber 到 MP 管道
        drawPipe(510, 250, 510, 270, 8);
        drawPipe(510, 295, 290, 295, 8);

        // Chamber 到 TP 管道
        drawPipe(560, 250, 560, 265, 8);
        drawPipe(730, 267, 730, 267, 8);

        // RF 连接管道
        drawPipe(400, 145, 455, 145, 6);

        // ICP 连接管道
        drawPipe(540, 70, 540, 85, 6);

        // 气体管道 (从MFC到Chamber)
        drawPipe(75, 40, 75, 140, 5);
        drawPipe(75, 140, 440, 140, 5);

        // MFC 出口到主管道
        for (int i = 0; i < 4; i++) {
            int y = 40 + i * 45;
            drawPipe(70, y, 75, y, 5);
        }

        // ===== 阀门 =====
        // V_LL (Load Lock阀)
        drawValveBody(260, 82, 25, 25);

        // V1 (主腔室到泵阀)
        drawValveBody(498, 240, 25, 25);

        // V2 (备用阀)
        drawValveBody(565, 282, 25, 25);

        // V_GAS (气体阀)
        drawValveBody(63, 127, 20, 25);

        // ===== 压力计 Pirani (右侧) =====
        QLinearGradient gaugeGrad;
        gaugeGrad.setStart(0, 0);
        gaugeGrad.setFinalStop(1, 0);
        gaugeGrad.setColorAt(0, QColor(150, 150, 155));
        gaugeGrad.setColorAt(0.5, QColor(120, 120, 125));
        gaugeGrad.setColorAt(1, QColor(90, 90, 95));

        QGraphicsRectItem *gauge = m_scene->addRect(900, 120, 95, 85);
        gauge->setBrush(QBrush(gaugeGrad));
        gauge->setPen(QPen(QColor(50, 50, 60), 2));

        // 压力显示
        QGraphicsRectItem *gaugeDisplay = m_scene->addRect(905, 125, 85, 30);
        gaugeDisplay->setBrush(QBrush(Qt::black));
        gaugeDisplay->setPen(Qt::NoPen);

        QGraphicsTextItem *pressVal = new QGraphicsTextItem("0.00");
        pressVal->setFont(QFont("Courier New", 16));
        pressVal->setPos(925, 128);
        pressVal->setDefaultTextColor(C_LED_GREEN);
        m_scene->addItem(pressVal);

        QGraphicsSimpleTextItem *mbarLabel = new QGraphicsSimpleTextItem("mbar");
        mbarLabel->setFont(QFont("Arial", 9));
        mbarLabel->setPos(935, 158);
        m_scene->addItem(mbarLabel);

        QGraphicsSimpleTextItem *piraniLabel = new QGraphicsSimpleTextItem("Pirani Gauge");
        piraniLabel->setFont(QFont("Arial", 8, QFont::Bold));
        piraniLabel->setPos(908, 178);
        m_scene->addItem(piraniLabel);

        // 压力计指示灯
        addLedIndicator(910, 192, true, C_LED_GREEN);

        // ===== Turbo 状态面板 (右上角) =====
        QGraphicsRectItem *turboPanel = m_scene->addRect(900, 10, 95, 75);
        turboPanel->setBrush(QBrush(QColor(80, 80, 90)));
        turboPanel->setPen(QPen(QColor(50, 50, 60), 2));

        QGraphicsSimpleTextItem *turboTitle = new QGraphicsSimpleTextItem("TURBO");
        turboTitle->setFont(QFont("Arial", 9, QFont::Bold));
        turboTitle->setPos(935, 15);
        m_scene->addItem(turboTitle);

        // 转速显示
        QGraphicsRectItem *turboDisplay = m_scene->addRect(905, 32, 50, 18);
        turboDisplay->setBrush(QBrush(Qt::black));
        turboDisplay->setPen(Qt::NoPen);

        QGraphicsTextItem *rpmVal = new QGraphicsTextItem("45000");
        rpmVal->setFont(QFont("Courier New", 10));
        rpmVal->setPos(908, 35);
        rpmVal->setDefaultTextColor(C_LED_GREEN);
        m_scene->addItem(rpmVal);

        QGraphicsSimpleTextItem *rpmLabel = new QGraphicsSimpleTextItem("RPM");
        rpmLabel->setFont(QFont("Arial", 7));
        rpmLabel->setPos(960, 38);
        m_scene->addItem(rpmLabel);

        // 状态指示
        addLedIndicator(908, 55, true, C_LED_GREEN);
        QGraphicsSimpleTextItem *atSpeed = new QGraphicsSimpleTextItem("at speed");
        atSpeed->setFont(QFont("Arial", 8));
        atSpeed->setPos(922, 57);
        m_scene->addItem(atSpeed);

        // N2吹扫
        QGraphicsSimpleTextItem *n2Label = new QGraphicsSimpleTextItem("N2 purge:");
        n2Label->setFont(QFont("Arial", 7));
        n2Label->setPos(908, 72);
        m_scene->addItem(n2Label);

        QGraphicsSimpleTextItem *n2Val = new QGraphicsSimpleTextItem("0.0 sccm");
        n2Val->setFont(QFont("Courier New", 8));
        n2Val->setPos(960, 72);
        m_scene->addItem(n2Val);

        // ===== 状态横幅 =====
        QGraphicsRectItem *banner = m_scene->addRect(165, 10, 260, 28);
        banner->setBrush(QBrush(C_TITLE_BLUE));
        banner->setPen(Qt::NoPen);

        QGraphicsTextItem *bannerText = new QGraphicsTextItem("Robot arm standing by");
        bannerText->setFont(QFont("Arial", 11));
        bannerText->setPos(175, 15);
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