#include "mask.h"

#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QMouseEvent>
#include <QResizeEvent>

void Mask::setColor(const QColor &color, float opacity)
{
    if (!color.isValid())
        return;

    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, color);
    this->setPalette(palette);
    this->setWindowOpacity(opacity);

    this->m_topWidget->installEventFilter(this);
}

Mask::Mask(QWidget* parent)
    : BorderlessWidget(parent), m_topWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    this->setColor(QColor(0, 0, 0), 0.6f);
    this->canMove(false);
}

Mask::~Mask() {}

void Mask::setTopWidget(QWidget *widget)
{
    this->m_topWidget = widget;
}

void Mask::install(QWidget *widget)
{
    Q_ASSERT(this->m_topWidget);

    if (!widget)
        return;

    widget->installEventFilter(this);
    this->m_popWidget.insert(widget);

    connect(widget, &QWidget::destroyed, [this, widget] () {
        this->m_popWidget.remove(widget);
    });
}

void Mask::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    this->setGeometry(m_topWidget->geometry());
}

bool Mask::eventFilter(QObject* watched, QEvent* event)
{
    if (this->m_popWidget.contains(dynamic_cast<QWidget*>(watched))) {
        switch (event->type())
        {
        case QEvent::Show:
            this->show();
            break;
        case QEvent::Hide:
            this->hide();
            break;
        default:
            break;
        }
    }
    else if (watched == this->m_topWidget) {
        if (event->type() == QEvent::Move)
            this->move(static_cast<QMoveEvent*>(event)->pos());
        else if (event->type() == QEvent::Resize)
            this->resize(static_cast<QResizeEvent*>(event)->size());
    }

    return QObject::eventFilter(watched, event);
}
