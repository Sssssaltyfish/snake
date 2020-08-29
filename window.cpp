#include <QtDebug>
#include <QWidget>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPushButton>
#include <QToolButton>
#include <QSpacerItem>
#include <QMessageBox>
#include <QPicture>
#include <QPainter>
#include <QRgb>
#include <QLabel>
#include <QLayout>
#include <QAction>

#include "window.h"
#include "ui_window.h"
#include "snake.h"

Window::Window(QWidget *parent)
    : QMainWindow(parent), mask(new Mask(this)), ui(new Ui::Window)
{
    setWindowIcon(QIcon("://image/icon/icon.png"));
    setWindowTitle("Greedy Snake");
    setObjectName("MainWindow");
    resize(1000, 900);
    init();
}

void Window::init()
{
    auto centralWidget = new QWidget(this);
    auto layout = new QVBoxLayout(centralWidget);
    centralWidget->setLayout(layout);

    mainWidget = new QWidget(centralWidget);
    ui->setupUi(mainWidget);
    {
        playGround = new PlayGround(mainWidget);
        playGround->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
        playGround->setFocusPolicy(Qt::StrongFocus);
        auto layout = dynamic_cast<QHBoxLayout*>(ui->mainWidget->layout());
        layout->insertItem(0, new QSpacerItem(0, 0, QSizePolicy::Minimum));
        layout->insertWidget(0, playGround);
        layout->insertItem(0, new QSpacerItem(0, 0, QSizePolicy::Minimum));
    }
    layout->addWidget(mainWidget);

#define ADD_ACTION(name, display)\
    auto name = new QAction(QIcon("://image/icon/"#name".png"), #display, playGround);\
    actions[#name] = name;
#define CONNECT(name, target)\
    connect(name, &QAction::triggered, playGround, &PlayGround::target)
#define ADD_AND_CONNECT(name, display)\
    ADD_ACTION(name, display);\
    CONNECT(name, name);\
    ui->name->setDefaultAction(name);\
    ui->name->setToolButtonStyle(Qt::ToolButtonTextOnly);\
    ui->name->setFocusPolicy(Qt::NoFocus);

    {
        ADD_AND_CONNECT(pause, 暂停游戏)
        ADD_AND_CONNECT(start, 开始游戏)
        ADD_AND_CONNECT(save, 保存游戏)
        ADD_AND_CONNECT(load, 加载游戏)
        ADD_AND_CONNECT(restart, 重新开始)
        ADD_AND_CONNECT(exit, 退出游戏)
        ADD_AND_CONNECT(cont, 继续游戏)
        pause->setShortcut(QKeySequence("P"));
        restart->setShortcut(QKeySequence("R"));
        exit->setShortcut(QKeySequence("Esc"));
    }

#undef ADD_AND_CONNECT
#undef CONNECT
#undef ADD_ACTION

    toolBar = new QToolBar();
    toolBar->setMovable(false);
    toolBar->addActions(getActions());
    toolBar->setFocusPolicy(Qt::NoFocus);
    layout->insertWidget(0, toolBar);

    menuBar = new QMenuBar();
    {
        auto manipMenu = new QMenu("操作", menuBar);
        manipMenu->addActions(getActions());
        menuBar->addMenu(manipMenu);
    }
    {
        auto helpMenu = new QMenu("接下来还有谁要做贪吃蛇", menuBar);
        auto icon = QPixmap("://image/icon/WHAT.png");
        auto helpAction = new QAction(icon, "我要", helpMenu);
        helpMenu->addAction(helpAction);

        info = new QWidget();
        info->setWhatsThis("I wrote this!");
        info->setWindowTitle("*他来了*");
        info->setWindowIcon(icon);
        //info->setWindowFlags(Qt::FramelessWindowHint);
        auto infoLayout = new QVBoxLayout(info);
        info->setLayout(infoLayout);
        auto label = new QLabel(info);
        label->setPixmap(QPixmap("://image/icon/u00bf.png"));
        label->setAlignment(Qt::AlignHCenter);
        infoLayout->addWidget(label);
        label = new QLabel("SssssaltyFish, 2020", info);
        label->setFont(QFont("Consolas", 10));
        label->setAlignment(Qt::AlignHCenter);
        infoLayout->addWidget(label);
        auto button = new QPushButton("*他的上升*", info);
        infoLayout->addWidget(button);
        connect(button, &QPushButton::clicked, info, &QWidget::close);
        connect(helpAction, &QAction::triggered, info, &QWidget::show);
        mask->install(info);
        menuBar->addMenu(helpMenu);
    }
    setMenuBar(menuBar);

    {
        youDied = new QWidget();
        youDied->setWhatsThis("你死了");
        youDied->setWindowTitle("死了啦");
        //youDied->setWindowFlags(Qt::FramelessWindowHint);
        youDied->setWindowIcon(QIcon("://image/icon/WHAT.png"));
        auto layout = new QVBoxLayout(youDied);
        youDied->setLayout(layout);
        auto label = new QLabel(youDied);
        label->setText("死了啦都怪你啦");
        label->setFont(QFont("黑体", 50, QFont::Bold));
        label->setAlignment(Qt::AlignCenter);
        auto button = new QPushButton(youDied);
        button->setText("我超勇的");
        layout->addWidget(label);
        layout->addWidget(button);
        connect(button, &QPushButton::clicked, youDied, &QWidget::close);
        connect(playGround, &PlayGround::stateUpdated, [=] (State state) {
            if (state == State::Died)
                youDied->show();
        });
        mask->install(youDied);
    }

    connect(playGround, SIGNAL(timeUpdated(int)), ui->time, SLOT(display(int)));
    connect(playGround, SIGNAL(scoreUpdated(int)), ui->score, SLOT(display(int)));
    connect(playGround, SIGNAL(stateUpdated(State)), this, SLOT(manageActions(State)));
    manageActions(playGround->getState());

    setCentralWidget(centralWidget);
}

void Window::manageActions(State state)
{
    static constexpr auto manager = std::array {
        uint8_t(0b01100001),
        uint8_t(0b01000010),
        uint8_t(0b01011100),
        uint8_t(0b01001000),
    };
    char mask {};
    switch (state) {
    case State::BeforeStart:
        mask = manager[0];
        break;
    case State::Active:
        mask = manager[1];
        break;
    case State::Paused:
        mask = manager[2];
        break;
    case State::Died:
        mask = manager[3];
        break;
    default:
        break;
    }
    for (auto&& action : getActions()) {
        action->setEnabled(mask & 1);
        mask >>= 1;
    }
}

QList<QAction*> Window::getActions()
{
    static constexpr auto sequence = std::array {
        "start",
        "pause",
        "cont",
        "restart",
        "save",
        "load",
        "exit",
    };

    QList<QAction*> ret;
    for (auto&& name : sequence)
        ret.append(actions[name]);
    return ret;
}

Window::~Window()
{
    delete ui;
    for (auto&& action : actions.values())
        delete action;
    delete info;
    delete youDied;
}

void Window::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    QCoreApplication::quit();
}
