#ifndef SNAKE_H
#define SNAKE_H

#include <utility>
#include <memory>
#include <QList>
#include <QSet>
#include <QPoint>
#include <QTimer>
#include <QJsonObject>
#include <QWidget>
#include <QRandomGenerator>

#include "mask.h"

enum Direction: uint8_t {
    Up,
    Right,
    Down,
    Left,
    NoDirection,
};

enum State: uint8_t {
    Paused,
    Active,
    BeforeStart,
    Died,
};

class Snake
{
public:
    Snake() = default;
    ~Snake() = default;

    bool move(Direction direction, bool longer = false);

private:
    QList<QPoint> body;
    QPoint head;
    Direction direction = Right;

    friend class Status;
    friend class PlayGround;
};

class Status
{
public:
    static Status fromJson(const QJsonObject& json);

public:
    Status(bool plain = false);
    ~Status() = default;

    QJsonObject toJson() const;
    QSet<QPoint> restNodes() const;

private:
    QSet<QPoint> obstacles;
    QSet<QPoint> foods;
    int currentTime = 0;
    Snake snake;
    int score = 0;
    int timerInterval = 200;
    static inline constexpr std::pair<uint8_t, uint8_t> boardSize { 42, 42 };

    friend class PlayGround;
};

class PlayGround : public QFrame
{
    Q_OBJECT

public:
    PlayGround(QWidget* parent);
    ~PlayGround();

    State getState() const;
    Status getStatus() const;

signals:
    void stateUpdated(State);
    void timeUpdated(int);
    void scoreUpdated(int);

public slots:
    void update();

    void pause();
    void cont();
    void start();
    void save();
    void load();
    void exit();
    void restart();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateStatus();
    QRect getRect(const QPoint& upLeftPoint);
    template <template <class> class _ContT>
    QVector<QRect> getRects(const _ContT<QPoint>& container);
    QPoint getGridPoint(const QPoint& point);
    Direction getDirection();
    void newFood();

private:
    QTimer timer;
    std::unique_ptr<Status> status;

    State state = BeforeStart;
    QPoint mousePosition;
    Direction futureDirection = NoDirection;
};

#endif // SNAKE_H
