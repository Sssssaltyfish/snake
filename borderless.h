#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>

class BorderlessWidget: public QMainWindow
{
    Q_OBJECT
public:
    explicit BorderlessWidget(QWidget *parent = nullptr);
    ~BorderlessWidget() {}

    void canMove(bool flag);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

protected slots:
    virtual void mouseMove(QMouseEvent* event);
    virtual void mousePress(QMouseEvent* event);
    virtual void mouseRelease(QMouseEvent* event);
    virtual void mouseDoubleClick(QMouseEvent* event);

protected slots:
    virtual void slot_minimized();
    virtual void slot_restored();
    virtual void slot_maximized();
    virtual void slot_closed();

private:
    bool mCanMove = true;
    bool mMoving = false;
    bool mMaximized = false;
    QPoint mLastMousePosition;
};
/*
class BorderlessMainWindow: public BorderlessWidget
{
    Q_OBJECT

public:
    explicit BorderlessMainWindow(QWidget *parent = nullptr, QMainWindow *mainWindow = new QMainWindow());
    ~BorderlessMainWindow() {}

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

protected slots:
    void slot_maximized() override;
    void slot_restored() override;

protected:
    QMainWindow *mMainWindow;
    QFrame *mTitlebarWidget;
    QLabel *mWindowTitle;
    QPushButton *mMinimizeButton;
    QPushButton *mRestoreButton;
    QPushButton *mMaximizeButton;
    QPushButton *mCloseButton;
    QLabel *mWindowIcon;
};
*/
#endif // MAINWINDOW_H
