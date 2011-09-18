/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "globalsearchtooltip.h"
#include "tooltipresultwidget.h"
#include "core/logging.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QLayoutItem>
#include <QPainter>
#include <QVBoxLayout>

const qreal GlobalSearchTooltip::kBorderRadius = 8.0;
const qreal GlobalSearchTooltip::kBorderWidth = 4.0;
const qreal GlobalSearchTooltip::kArrowWidth = 10.0;
const qreal GlobalSearchTooltip::kArrowHeight = 10.0;


GlobalSearchTooltip::GlobalSearchTooltip(QObject* event_target)
  : QWidget(NULL),
    desktop_(qApp->desktop()),
    event_target_(event_target)
{
  setWindowFlags(Qt::Popup);
  setFocusPolicy(Qt::NoFocus);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_TranslucentBackground);
}

void GlobalSearchTooltip::SetResults(const SearchProvider::ResultList& results) {
  results_ = results;

  qDeleteAll(widgets_);
  widgets_.clear();

  // Using a QVBoxLayout here made some weird flickering that I couldn't figure
  // out how to fix, so do layout manually.
  int y = 9;
  int w = 0;

  foreach (const SearchProvider::Result& result, results) {
    QWidget* widget = new TooltipResultWidget(result, this);
    widget->move(0, y);
    widget->show();
    widgets_ << widget;

    QSize size_hint(widget->sizeHint());
    y += size_hint.height();
    w = qMax(w, size_hint.width());
  }

  foreach (QWidget* widget, widgets_) {
    widget->resize(w, widget->sizeHint().height());
  }

  y += 9;

  resize(w, y);

  inner_rect_ = rect().adjusted(
        kArrowWidth + kBorderWidth, kBorderWidth, -kBorderWidth, -kBorderWidth);

  foreach (QWidget* widget, widgets_) {
    widget->setMask(inner_rect_);
  }
}

void GlobalSearchTooltip::ShowAt(const QPoint& pointing_to) {
  const qreal min_arrow_offset = kBorderRadius + kArrowHeight;
  const QRect screen = desktop_->screenGeometry(this);

  arrow_offset_ = min_arrow_offset +
                  qMax(0, pointing_to.y() + height() - screen.bottom());

  move(pointing_to.x(), pointing_to.y() - arrow_offset_);

  if (!isVisible())
    show();
}

void GlobalSearchTooltip::keyPressEvent(QKeyEvent* e) {
  // Copy the event to send to the target
  QKeyEvent copy(e->type(), e->key(), e->modifiers(), e->text(),
                 e->isAutoRepeat(), e->count());

  qApp->sendEvent(event_target_, &copy);

  e->accept();
}

void GlobalSearchTooltip::paintEvent(QPaintEvent*) {
  QPainter p(this);

  QColor color = Qt::black;

  // Transparent background
  p.fillRect(rect(), Qt::transparent);

  QRect area(inner_rect_.adjusted(
      -kBorderWidth/2, -kBorderWidth/2, kBorderWidth/2, kBorderWidth/2));

  // Draw the border
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(QPen(color, kBorderWidth));
  p.setBrush(palette().color(QPalette::Base));
  p.drawRoundedRect(area, kBorderRadius, kBorderRadius);

  // Draw the arrow
  QPolygonF arrow;
  arrow << QPointF(kArrowWidth, arrow_offset_ - kArrowHeight)
        << QPointF(0, arrow_offset_)
        << QPointF(kArrowWidth, arrow_offset_ + kArrowHeight);

  p.setBrush(color);
  p.setPen(color);
  p.drawPolygon(arrow);
}
