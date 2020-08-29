#include <utility>
#include <QJsonDocument>
#include <QResizeEvent>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonArray>
#include <QIODevice>
#include <QVariant>
#include <QPainter>
#include <QDebug>
#include <QTime>
#include <QPen>

#include "snake.h"

// * Right: x+
// * Down: y+

inline uint qHash(const QPoint& key)
{
    return qHash(QPair { key.x(), key.y() });
}

QString node2str(const QPoint& node)
{
    return QString::number(node.x()) + " " + QString::number(node.y());
}

QPoint str2node(const QString& str)
{
    if (str.isEmpty())
        return QPoint();

    auto list = str.split(' ');
    auto x = list[0].toInt();
    auto y = list[1].toInt();
    return QPoint(x, y);
}

QPoint moved(const QPoint& head, Direction direction)
{
    QPoint ret { head };
    switch (direction) {
    case Direction::Up:
        ret.ry() -= 1;
        break;
    case Direction::Right:
        ret.rx() += 1;
        break;
    case Direction::Down:
        ret.ry() += 1;
        break;
    case Direction::Left:
        ret.rx() -= 1;
        break;
    default:
        break;
    }
    return ret;
}

bool Snake::move(Direction _direction, bool _longer)
{
    static unsigned rest = 0;
    auto newHead = moved(head, _direction);
    for (auto&& node : body)
        if (node == newHead)
            return false;

    direction = _direction;
    body.push_back(head);
    head = newHead;
    if (!_longer) {
        if (rest)
            --rest;
        else
            body.pop_front();
    }
    else
        rest += 2;
    return true;
}

Status::Status(bool plain)
{
    if (plain)
        return;

    static_assert(boardSize.first > 3);
    static_assert(boardSize.second > 3);
    auto& x = boardSize.first;
    auto& y = boardSize.second;
    for (auto i = 0; i < y; ++i) {
        obstacles.insert({0, i});
        obstacles.insert({x - 1, i});
    }
    for (auto j = 1; j < x - 1; ++j) {
        obstacles.insert({j, y - 1});
        obstacles.insert({j, 0});
    }
    snake.head = {x / 2, y / 2};
    snake.body.append({x / 2 - 1, y / 2});
}

QJsonObject Status::toJson() const
{
#define INSERT(val) ret.insert(#val, val)

    QJsonObject ret;
    INSERT(currentTime);
    INSERT(score);
    INSERT(timerInterval);
    QJsonArray obstacles;
    for (auto&& node : this->obstacles)
        obstacles.append(node2str(node));
    INSERT(obstacles);
    QJsonArray foods;
    for (auto&& node : this->foods)
        foods.append(node2str(node));
    INSERT(foods);
    QJsonObject snake;
    QJsonArray body;
    for (auto&& node : this->snake.body)
        body.append(node2str(node));
    snake.insert("body", body);
    snake.insert("head", node2str(this->snake.head));
    snake.insert("direction", this->snake.direction);
    INSERT(snake);
    return ret;

#undef INSERT
}

Status Status::fromJson(const QJsonObject &json)
{
#define GET(val) ret.val = json[#val]
#define VALUE(val) decltype(Status::val)& val = ret.val
#define FORNODE(val) VALUE(val); for (auto&& str : json[#val].toArray()) val.insert(str2node(str.toString()));

    Status ret(true);
    GET(currentTime).toInt(ret.currentTime);
    GET(score).toInt(ret.score);
    GET(timerInterval).toInt(ret.timerInterval);
    FORNODE(obstacles);
    FORNODE(foods);
    auto snakeJson = json["snake"];
    ret.snake.head = str2node(snakeJson["head"].toString());
    ret.snake.direction = Direction(snakeJson["direction"].toInt(ret.snake.direction));
    for (auto&& str : snakeJson["body"].toArray())
        ret.snake.body.append(str2node(str.toString()));
    return ret;

#undef FORNODE
#undef VALUE
#undef GET
}

QSet<QPoint> Status::restNodes() const
{
    QSet<QPoint> ret;
    for (auto i = 0; i < boardSize.first; ++i)
        for (auto j = 0; j < boardSize.second; ++j)
            ret.insert({i, j});
    for (auto&& node : obstacles)
        ret.remove(node);
    for (auto&& node : snake.body)
        ret.remove(node);
    for (auto&& node : foods)
        ret.remove(node);
    ret.remove(snake.head);
    return ret;
}

PlayGround::PlayGround(QWidget* parent)
    : QFrame(parent),
      status(new Status()),
      mousePosition(Status::boardSize.first, Status::boardSize.second)
{
    timer.callOnTimeout(this, &PlayGround::update);
    setFrameStyle(QFrame::Box | QFrame::Sunken);
    setMouseTracking(true);
    setMinimumSize(500, 500);
    updateStatus();
}

PlayGround::~PlayGround()
{}

void PlayGround::newFood()
{
    auto rest = status->restNodes();
    auto pos = QRandomGenerator::global()->bounded(rest.size());
    status->foods.insert(*(rest.begin() + pos));
}

void PlayGround::start()
{
    state = State::Active;
    newFood();
    mousePosition = QPoint(Status::boardSize.first, Status::boardSize.second);
    updateStatus();
    timer.start();
}

void PlayGround::exit()
{
    QCoreApplication::exit();
}

void PlayGround::pause()
{
    state = State::Paused;
    updateStatus();
    timer.stop();
}

void PlayGround::cont()
{
    state = State::Active;
    updateStatus();
    timer.start();
}

void PlayGround::save()
{
    auto path = QFileDialog::getSaveFileName(this, "保存", ".", "JSON(*.json)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return; // File is automatically closed in destructor. No need for manual closing.

    QJsonDocument jsonDoc(status->toJson());
    file.write(jsonDoc.toJson());
}

void PlayGround::load()
{
    auto path = QFileDialog::getOpenFileName(this, "打开", ".", "JSON(*.json)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return; // See function `void PlayGround::save()`.

    auto data = file.readAll();
    file.close();
    QJsonParseError jsonError;
    auto jsonDoc = QJsonDocument::fromJson(data, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "错误", "并非有效的 json 文件");
        return;
    }
    auto newStatus = new auto { Status::fromJson(jsonDoc.object()) };
    status.reset(newStatus);

    mousePosition = QPoint(Status::boardSize.first, Status::boardSize.second);
    state = State::Paused;
    updateStatus();
}

void PlayGround::restart()
{
    state = State::BeforeStart;
    status.reset(new Status());
    mousePosition = QPoint(Status::boardSize.first, Status::boardSize.second);
    updateStatus();
    timer.stop();
}

void PlayGround::updateStatus()
{
    timer.setInterval(status->timerInterval);
    emit timeUpdated(status->currentTime);
    emit scoreUpdated(status->score);
    emit stateUpdated(state);
    repaint();
}

void PlayGround::paintEvent(QPaintEvent *event)
{
    static auto headBrush = QBrush(QColor(0xe9, 0x1e, 0x63));
    static auto foodBrush = QBrush(QColor(0xff, 0xc1, 0x07));
    static auto obstacleBrush = QBrush(QColor(0x00, 0x00, 0x00));
    static auto bodyBrush = QBrush(QColor(0x4c, 0xaf, 0x50));
    static auto mouseBrush = QBrush(QColor(0x00, 0x00, 0x00, 255/2));
    static auto pen = QPen(QColor("transparent"), 20);

    QPainter painter(this);
    painter.setPen(pen);

    painter.setBrush(foodBrush);
    painter.drawRects(getRects(status->foods));

    painter.setBrush(headBrush);
    painter.drawRect(getRect(status->snake.head));

    painter.setBrush(bodyBrush);
    painter.drawRects(getRects(status->snake.body));

    painter.setBrush(obstacleBrush);
    painter.drawRects(getRects(status->obstacles));

    painter.setBrush(mouseBrush);
    painter.drawRect(getRect(mousePosition));

    QFrame::paintEvent(event);
}

void PlayGround::update()
{
    ++status->currentTime;
    status->timerInterval = 5 + status->timerInterval * 0.95;

    auto& snake = status->snake;
    {
        auto direction = getDirection();
        auto movedHead = moved(snake.head, direction);
        qDebug() << movedHead << status->foods << direction;
        auto moveFlag = true;
        if (status->obstacles.contains(movedHead)) {
                moveFlag = false;
        }
        else if (status->foods.contains(movedHead)) {
            status->foods.remove(movedHead);
            newFood();
            moveFlag = snake.move(direction, true);
            status->score += 1;
        }
        else {
            moveFlag = snake.move(direction);
        }
        if (!moveFlag) {
            timer.stop();
            state = State::Died;
        }
    }

    updateStatus();
}

Direction PlayGround::getDirection()
{
    auto originalDirection = status->snake.direction;
    if (futureDirection == Direction::NoDirection)
        return originalDirection;

    if ((futureDirection + originalDirection) % 2) {
        originalDirection = futureDirection;
    }
    futureDirection = Direction::NoDirection;
    return originalDirection;
}

State PlayGround::getState() const
{
    return state;
}

Status PlayGround::getStatus() const
{
    return *status;
}

QRect PlayGround::getRect(const QPoint &upLeftPoint)
{
    static constexpr auto sizeRatio = 0.9f, marginRatio = (1 - sizeRatio) / sizeRatio;
    auto rect = this->rect();
    auto x = rect.right(), y = rect.bottom();
    auto xGrid = status->boardSize.first, yGrid = status->boardSize.second;
    int
        up = (double)upLeftPoint.x() / xGrid * x,
        left = (double)upLeftPoint.y() / yGrid * y;
    int xMargin = x * sizeRatio / xGrid, yMargin = y * sizeRatio / yGrid;
    return QRect(up + yMargin * marginRatio,left + xMargin * marginRatio, yMargin, xMargin);
}

template <template <class> class _ContT>
QVector<QRect> PlayGround::getRects(const _ContT<QPoint>& container)
{
    QVector<QRect> ret;
    for (auto&& point : container)
        ret.append(getRect(point));
    return ret;
}

QPoint PlayGround::getGridPoint(const QPoint& point)
{
    auto rect = this->rect();
    auto x = rect.bottom(), y = rect.right();
    auto xGrid = status->boardSize.first, yGrid = status->boardSize.second;
    return QPoint(point.x() / ((double)x / xGrid), point.y() / ((double)y / yGrid));
}

void PlayGround::resizeEvent(QResizeEvent *event)
{
    auto newSize = event->size();
    auto finalSize = std::min({newSize.width(), newSize.height()});
    QFrame::resize(finalSize, finalSize);
    QFrame::resizeEvent(event);
}

void PlayGround::mousePressEvent(QMouseEvent *event)
{
    if (state != State::BeforeStart)
        return;

    auto point = getGridPoint(event->pos());
    for (auto&& node : status->restNodes()) {
        if (node == point) {
            status->obstacles.insert(node);
            repaint();
            return;
        }
    }
    for (auto&& node : status->obstacles) {
        if (node == point) {
            status->obstacles.remove(node);
            repaint();
            return;
        }
    }
}

void PlayGround::mouseMoveEvent(QMouseEvent *event)
{
    if (state != State::BeforeStart)
            return;

    mousePosition = getGridPoint(event->pos());
    repaint();
}

void PlayGround::keyPressEvent(QKeyEvent *event)
{
    if (state == State::Died)
        return;

    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        futureDirection = Direction::Up;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        futureDirection = Direction::Right;
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        futureDirection = Direction::Down;
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        futureDirection = Direction::Left;
        break;
    default:
        break;
    }

    if (state == State::BeforeStart)
        start();
    else if (state == State::Paused)
        cont();
}
