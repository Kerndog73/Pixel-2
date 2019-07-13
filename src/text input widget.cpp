//
//  text input widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "text input widget.hpp"

#include "connect.hpp"
#include <QtGui/qevent.h>
#include "global font.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"

TextInputWidget::TextInputWidget(QWidget *parent, const WidgetRect rect)
  : QLineEdit{parent},
    rect{rect},
    cursorBlinkTimer{this} {
  setFixedSize(rect.widget().size());
  setFont(getGlobalFont());
  updateMargins();
  setFrame(false);
  setAttribute(Qt::WA_MacShowFocusRect, 0);
  
  CONNECT(cursorBlinkTimer, timeout,               this, blink);
  CONNECT(this,             selectionChanged,      this, showCursor);
  CONNECT(this,             cursorPositionChanged, this, setOffset);
  CONNECT(this,             cursorPositionChanged, this, showCursor);
  
  cursorBlinkTimer.setInterval(box_cursor_blink_interval_ms);
  cursorBlinkTimer.setTimerType(Qt::CoarseTimer);
  cursorBlinkTimer.start();
}

void TextInputWidget::blink() {
  cursorBlinkStatus = !cursorBlinkStatus;
  repaint();
}

void TextInputWidget::showCursor() {
  cursorBlinkStatus = true;
  cursorBlinkTimer.stop();
  cursorBlinkTimer.start();
  repaint();
}

void TextInputWidget::hideCursor() {
  cursorBlinkStatus = false;
  cursorBlinkTimer.stop();
  cursorBlinkTimer.start();
  repaint();
}

void TextInputWidget::focusInEvent(QFocusEvent *event) {
  QLineEdit::focusInEvent(event);
  hideCursor();
  QTimer::singleShot(0, this, &QLineEdit::selectAll);
}

void TextInputWidget::focusOutEvent(QFocusEvent *event) {
  offset = 0;
  QLineEdit::focusOutEvent(event);
}

void TextInputWidget::wheelEvent(QWheelEvent *event) {
  offset += event->pixelDelta().x();
  constrainOffset();
  updateMargins();
  repaint();
}

int TextInputWidget::getCursorPos(const int chars) const {
  return rect.contentPos().x()
    + chars * glob_font_stride_px
    - glob_text_padding
    + offset;
}

int TextInputWidget::getMinCursorPos() const {
  return rect.contentPos().x() - glob_text_padding;
}

int TextInputWidget::getMaxCursorPos() const {
  return rect.widgetSpace() + rect.outer().width() - glob_text_padding;
}

void TextInputWidget::setOffset(int, const int newCursor) {
  const int newPos = getCursorPos(newCursor);
  const int min = getMinCursorPos();
  const int max = getMaxCursorPos();
  if (newPos < min) {
    offset += min - newPos;
  } else if (newPos > max) {
    offset -= newPos - max;
  }
  constrainOffset();
  updateMargins();
}

void TextInputWidget::constrainOffset() {
  const int max = getMaxCursorPos();
  offset = std::max(offset, max - text().length() * glob_font_stride_px);
  offset = std::min(offset, 0);
}

void TextInputWidget::updateMargins() {
  setTextMargins(
    rect.contentPos().x() + offset,
    rect.contentPos().y(),
    rect.inner().x(),
    rect.inner().y()
  );
}

void TextInputWidget::renderBackground(QPainter &painter) {
  painter.setBrush(box_background_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(rect.inner());
}

void TextInputWidget::renderText(QPainter &painter) {
  painter.setFont(getGlobalFont());
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  painter.setClipRect(rect.inner());
  QPoint textPos = rect.contentPos();
  textPos.rx() += offset;
  textPos.ry() += glob_font_accent_px;
  painter.drawText(textPos, text());
}

void TextInputWidget::renderCursor(QPainter &painter) {
  if (!hasFocus() || !cursorBlinkStatus || selectionStart() != -1) return;
  painter.setBrush(glob_text_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(
    getCursorPos(cursorPosition()),
    rect.inner().y(),
    box_cursor_width,
    rect.inner().height()
  );
}

void TextInputWidget::renderSelection(QPainter &painter) {
  if (!hasFocus() || selectionStart() == -1) return;
  painter.setBrush(box_selection_color);
  painter.setPen(Qt::NoPen);
  painter.drawRect(
    getCursorPos(selectionStart()),
    rect.inner().y(),
    selectionLength() * glob_font_stride_px + glob_font_kern_px,
    rect.inner().height()
  );
}

void TextInputWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  renderBackground(painter);
  paintBorder(painter, rect, glob_border_color);
  renderText(painter);
  renderCursor(painter);
  renderSelection(painter);
}

#include "text input widget.moc"
