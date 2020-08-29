#include <QDebug>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QIcon>
#include <QPicture>

#include "borderless.h"

BorderlessWidget::BorderlessWidget(QWidget *parent) : QMainWindow(parent, Qt::CustomizeWindowHint ) {
    setObjectName("borderlessMainWindow");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
}
/*
BorderlessMainWindow::BorderlessMainWindow(QWidget* parent, QMainWindow *mainWindow) : BorderlessWidget(parent) {
    mMainWindow = mainWindow;
    mMainWindow->setParent(this);
    setWindowTitle(mMainWindow->windowTitle());

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(0);
    verticalLayout->setMargin(1);

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(0);
    horizontalLayout->setMargin(0);

    mTitlebarWidget = new QFrame(this);
    mTitlebarWidget->setObjectName("titlebarWidget");
    mTitlebarWidget->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mTitlebarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mTitlebarWidget->setLayout(horizontalLayout);

    mMinimizeButton = new QPushButton(mTitlebarWidget);
    mMinimizeButton->setObjectName("minimizeButton");
    mMinimizeButton->setIcon(QIcon("://image/icon/minimize.png"));
    connect(mMinimizeButton, SIGNAL(clicked()), this, SLOT(slot_minimized()));

    mRestoreButton = new QPushButton(mTitlebarWidget);
    mRestoreButton->setObjectName("restoreButton");
    mRestoreButton->setVisible(false);
    mRestoreButton->setIcon(QIcon("://image/icon/restore.png"));
    connect(mRestoreButton, SIGNAL(clicked()), this, SLOT(slot_restored()));

    mMaximizeButton = new QPushButton(mTitlebarWidget);
    mMaximizeButton->setObjectName("maximizeButton");
    mMaximizeButton->setIcon(QIcon("://image/icon/maximize.png"));
    connect(mMaximizeButton, SIGNAL(clicked()), this, SLOT(slot_maximized()));

    mCloseButton = new QPushButton(mTitlebarWidget);
    mCloseButton->setObjectName("closeButton");
    mCloseButton->setIcon(QIcon("://image/icon/close.png"));
    connect(mCloseButton, SIGNAL(clicked()), this, SLOT(slot_closed()));

    mWindowTitle = new QLabel(mTitlebarWidget);
    mWindowTitle->setObjectName("windowTitle");
    mWindowTitle->setText(windowTitle());

    mWindowIcon = new QLabel(mTitlebarWidget);
    mWindowIcon->setObjectName("windowIcon");
    mWindowIcon->setPixmap(QPixmap("://image/icon/icon.png").scaled(mCloseButton->sizeHint()));

    horizontalLayout->addWidget(mWindowIcon);
    horizontalLayout->addWidget(mWindowTitle);
    horizontalLayout->addStretch(1);
    horizontalLayout->addWidget(mMinimizeButton);
    horizontalLayout->addWidget(mRestoreButton);
    horizontalLayout->addWidget(mMaximizeButton);
    horizontalLayout->addWidget(mCloseButton);

    verticalLayout->addWidget(mTitlebarWidget);
    verticalLayout->addWidget(mMainWindow);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    centralWidget->setLayout(verticalLayout);

    setCentralWidget(centralWidget);
}
*/
void BorderlessWidget::canMove(bool flag = true) {
    mCanMove = flag;
}

void BorderlessWidget::mousePress(QMouseEvent* event) {
    if (!mCanMove)
        return;

    if(event->button() == Qt::LeftButton) {
        mMoving = true;
        mLastMousePosition = event->pos();
    }
}

void BorderlessWidget::mousePressEvent(QMouseEvent* event) {
    mousePress(event);
}
/*
void BorderlessMainWindow::mousePressEvent(QMouseEvent *event) {
    if (!mTitlebarWidget->underMouse() && !mWindowTitle->underMouse())
        return;
    mousePress(event);
}
*/
void BorderlessWidget::mouseMove(QMouseEvent* event) {
    if (!mCanMove)
        return;

    if( event->buttons().testFlag(Qt::LeftButton) && mMoving) {
        this->move(this->pos() + (event->pos() - mLastMousePosition));
    }
}

void BorderlessWidget::mouseMoveEvent(QMouseEvent *event) {
    mouseMove(event);
}
/*
void BorderlessMainWindow::mouseMoveEvent(QMouseEvent* event) {
    if (!mTitlebarWidget->underMouse() && !mWindowTitle->underMouse())
        return;
    mouseMove(event);
}
*/
void BorderlessWidget::mouseRelease(QMouseEvent* event) {
    if (!mCanMove)
        return;

    if(event->button() == Qt::LeftButton) {
        mMoving = false;
    }
}

void BorderlessWidget::mouseReleaseEvent(QMouseEvent *event) {
    mouseRelease(event);
}
/*
void BorderlessMainWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (!mTitlebarWidget->underMouse() && !mWindowTitle->underMouse())
        return;
    mouseRelease(event);
}
*/
void BorderlessWidget::mouseDoubleClick(QMouseEvent* event) {
    Q_UNUSED(event);

    if (!mCanMove)
        return;

    mMaximized = !mMaximized;
    if (mMaximized) {
        slot_maximized();
    } else {
        slot_restored();
    }
}

void BorderlessWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    mouseDoubleClick(event);
}
/*
void BorderlessMainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    if (!mTitlebarWidget->underMouse() && !mWindowTitle->underMouse())
        return;
    mouseDoubleClick(event);
}
*/
void BorderlessWidget::slot_minimized() {
    setWindowState(Qt::WindowMinimized);
}

void BorderlessWidget::slot_restored() {
    setWindowState(Qt::WindowNoState);
    setStyleSheet("#borderlessMainWindow{border:1px solid palette(highlight);}");
}
/*
void BorderlessMainWindow::slot_restored() {
    mRestoreButton->setVisible(false);
    mMaximizeButton->setVisible(true);
    this->BorderlessWidget::slot_restored();
}
*/
void BorderlessWidget::slot_maximized() {
    setWindowState(Qt::WindowMaximized);
    setStyleSheet("#borderlessMainWindow{border:1px solid palette(base);}");
}
/*
void BorderlessMainWindow::slot_maximized() {
    mRestoreButton->setVisible(true);
    mMaximizeButton->setVisible(false);
    this->BorderlessWidget::slot_maximized();
}
*/
void BorderlessWidget::slot_closed() {
    close();
}
