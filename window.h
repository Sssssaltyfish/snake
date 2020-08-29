#pragma once

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QTimer>
#include <QToolBar>
#include <QAction>

#include "mask.h"
#include "snake.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Window; }
QT_END_NAMESPACE

class Window : public QMainWindow
{
    Q_OBJECT

    friend class Mask;

public:
    Window(QWidget *parent = nullptr);
    ~Window();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void init();
    QList<QAction*> getActions();

private slots:
    void manageActions(State state);

private:
    Mask* mask;
    Ui::Window* ui;
    QHash<QString, QAction*> actions;
    QToolBar* toolBar;
    QMenuBar* menuBar;
    QWidget* mainWidget;
    QWidget* info;
    QWidget* youDied;
    PlayGround* playGround;
};
#endif // WINDOW_H
