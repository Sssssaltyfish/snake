#pragma once

#ifndef MASK_H
#define MASK_H

#include <QWidget>
#include <QSet>

#include "borderless.h"

class Mask : public BorderlessWidget
{
    Q_OBJECT

public:
    Mask(QWidget* parent);
    ~Mask();

    void setColor(const QColor& color, float opacity);
    void setTopWidget(QWidget* widget);
    void install(QWidget* widget);

protected:
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget* m_topWidget;
    QSet<QWidget*> m_popWidget;
};

#endif // MASK_H
