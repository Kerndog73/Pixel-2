//
//  tool impls.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool impls.hpp"

#include <cmath>
#include "painting.hpp"
#include "composite.hpp"
#include "cell impls.hpp"

#include <iostream>

namespace {

QRgb selectColor(const ToolColors &colors, const ButtonType button) {
  switch (button) {
    case ButtonType::primary:   return colors.primary;
    case ButtonType::secondary: return colors.secondary;
    case ButtonType::erase:     return colors.erase;
    default: Q_UNREACHABLE();
  }
}

ToolChanges drawnChanges(const bool drawn) {
  return drawn ? ToolChanges::cell_overlay : ToolChanges::overlay;
}

std::string &operator+=(std::string &status, const QPoint point) {
  status += '[';
  status += std::to_string(point.x());
  status += ' ';
  status += std::to_string(point.y());
  status += ']';
  return status;
}

std::string &operator+=(std::string &status, const QSize size) {
  return status += QPoint{size.width(), size.height()};
}

void statusPos(std::string &status, const QPoint pos) {
  status += "POS: ";
  status += pos;
}

void statusPosSize(std::string &status, const QPoint pos, const QSize size) {
  statusPos(status, pos);
  status += " SIZE: ";
  status += size;
}

void statusMode(std::string &status, const SelectMode mode) {
  if (mode == SelectMode::copy) {
    status += "COPY - ";
  } else if (mode == SelectMode::paste) {
    status += "PASTE - ";
  } else Q_UNREACHABLE();
}

void statusBool(std::string &status, const bool b) {
  status += b ? "YES" : "NO";
}

}

bool BrushTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void BrushTool::detachCell() {
  assert(source);
  source = nullptr;
}

ToolChanges BrushTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  symPoint(*event.overlay, overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  lastPos = event.pos;
  color = selectColor(event.colors, event.button);
  return drawnChanges(symPoint(source->image.data, color, lastPos));
}

ToolChanges BrushTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  symPoint(*event.overlay, overlay_color, event.pos);
  symPoint(*event.status, event.pos);
  if (event.button == ButtonType::none) return ToolChanges::overlay;
  Image &img = source->image;
  const bool drawn = symLine(img.data, color, {lastPos, event.pos});
  lastPos = event.pos;
  return drawnChanges(drawn);
}

ToolChanges BrushTool::mouseUp(const ToolMouseEvent &event) {
  assert(source);
  symPoint(*event.status, event.pos);
  return drawnChanges(symLine(source->image.data, color, {lastPos, event.pos}));
}

void BrushTool::setWidth(const int newWidth) {
  assert(min_thickness <= newWidth && newWidth <= max_thickness);
  width = newWidth;
}

void BrushTool::setMode(const SymmetryMode newMode) {
  mode = newMode;
}

void BrushTool::symPoint(std::string &status, const QPoint point) {
  const QSize size = source->image.data.size();
  const QPoint refl = {size.width() - point.x() - 1, size.height() - point.y() - 1};
  status += "POS: ";
  status += point;
  if (mode & SymmetryMode::hori) {
    status += ' ';
    status += QPoint{refl.x(), point.y()};
  }
  if (mode & SymmetryMode::vert) {
    status += ' ';
    status += QPoint{point.x(), refl.y()};
  }
  if (mode & SymmetryMode::both) {
    status += ' ';
    status += refl;
  }
}

bool BrushTool::symPoint(QImage &img, const QRgb col, const QPoint point) {
  // @TODO symmetry isn't quite right when brush size isn't 1
  const QPoint refl = {img.width() - point.x() - 1, img.height() - point.y() - 1};
  bool drawn = drawRoundPoint(img, col, point, width);
  if (mode & SymmetryMode::hori) {
    drawn |= drawRoundPoint(img, col, {refl.x(), point.y()}, width);
  }
  if (mode & SymmetryMode::vert) {
    drawn |= drawRoundPoint(img, col, {point.x(), refl.y()}, width);
  }
  if (mode & SymmetryMode::both) {
    drawn |= drawRoundPoint(img, col, refl, width);
  }
  return drawn;
}

namespace {

QPoint reflectX(const QSize size, const QPoint point) {
  return {size.width() - point.x() - 1, point.y()};
}

QPoint reflectY(const QSize size, const QPoint point) {
  return {point.x(), size.height() - point.y() - 1};
}

QPoint reflectXY(const QSize size, const QPoint point) {
  return reflectX(size, reflectY(size, point));
}

}

bool BrushTool::symLine(QImage &img, const QRgb col, const QLine line) {
  const QSize size = img.size();
  bool drawn = drawRoundLine(img, col, line, width);
  if (mode & SymmetryMode::hori) {
    const QLine refl = {reflectX(size, line.p1()), reflectX(size, line.p2())};
    drawn |= drawRoundLine(img, col, refl, width);
  }
  if (mode & SymmetryMode::vert) {
    const QLine refl = {reflectY(size, line.p1()), reflectY(size, line.p2())};
    drawn |= drawRoundLine(img, col, refl, width);
  }
  if (mode & SymmetryMode::both) {
    const QLine refl = {reflectXY(size, line.p1()), reflectXY(size, line.p2())};
    drawn |= drawRoundLine(img, col, refl, width);
  }
  return drawn;
}

bool FloodFillTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void FloodFillTool::detachCell() {
  assert(source);
  source = nullptr;
}

ToolChanges FloodFillTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, overlay_color, event.pos);
  statusPos(*event.status, event.pos);
  const QRgb color = selectColor(event.colors, event.button);
  return drawnChanges(drawFloodFill(source->image.data, color, event.pos));
}

ToolChanges FloodFillTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  drawSquarePoint(*event.overlay, overlay_color, event.pos);
  statusPos(*event.status, event.pos);
  return ToolChanges::overlay;
}

bool RectangleSelectTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void RectangleSelectTool::detachCell() {
  assert(source);
  source = nullptr;
}

// @TODO maybe dragging a rectangle with secondary will cut instead of copy?

ToolChanges RectangleSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  statusMode(*event.status, mode);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, overlay_color, event.pos);
    statusPos(*event.status, event.pos);
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    statusPosSize(*event.status, event.pos + offset, overlay.size());
  } else Q_UNREACHABLE();
  if (event.button != ButtonType::primary) return ToolChanges::overlay;
  if (mode == SelectMode::copy) {
    startPos = event.pos;
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    blitImage(source->image.data, selection, event.pos + offset);
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges RectangleSelectTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  statusMode(*event.status, mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      const QRect rect = QRect{startPos, event.pos}.normalized();
      drawStrokedRect(*event.overlay, overlay_color, rect);
      statusPosSize(*event.status, rect.topLeft(), rect.size());
    } else {
      drawSquarePoint(*event.overlay, overlay_color, event.pos);
      statusPos(*event.status, event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    statusPosSize(*event.status, event.pos + offset, overlay.size());
  } else Q_UNREACHABLE();
  return ToolChanges::overlay;
}

ToolChanges RectangleSelectTool::mouseUp(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, overlay_color, event.pos);
    const QRect rect = QRect{startPos, event.pos}.normalized();
    selection = blitImage(source->image.data, rect);
    overlay = selection;
    colorToOverlay(overlay);
    offset = rect.topLeft() - event.pos;
    startPos = no_point;
    // @TODO maybe switch to paste mode here?
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    startPos = no_point;
    return ToolChanges::overlay;
  } else Q_UNREACHABLE();
}

ToolChanges RectangleSelectTool::keyPress(const ToolKeyEvent &event) {
  if (event.key == Qt::Key_P && startPos == no_point) {
    mode = opposite(mode);
  }
  statusMode(*event.status, mode);
  return ToolChanges::none;
}

void RectangleSelectTool::setMode(const SelectMode newMode) {
  mode = newMode;
}

bool MaskSelectTool::attachCell(Cell *cell) {
  return source = dynamic_cast<SourceCell *>(cell);
}

void MaskSelectTool::detachCell() {
  assert(source);
  source = nullptr;
}

ToolChanges MaskSelectTool::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  statusMode(*event.status, mode);
  if (mode == SelectMode::copy) {
    drawSquarePoint(*event.overlay, overlay_color, event.pos);
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
  } else Q_UNREACHABLE();
  if (event.button != ButtonType::primary) return ToolChanges::overlay;
  if (mode == SelectMode::copy) {
    polygon.clear();
    polygon.push_back(event.pos);
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    blitMaskImage(source->image.data, mask, selection, event.pos + offset);
    return ToolChanges::cell_overlay;
  } else Q_UNREACHABLE();
}

ToolChanges MaskSelectTool::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  statusMode(*event.status, mode);
  if (mode == SelectMode::copy) {
    if (event.button == ButtonType::primary) {
      polygon.push_back(event.pos);
      drawFilledPolygon(*event.overlay, overlay_color, polygon, QPoint{0, 0});
    } else {
      drawSquarePoint(*event.overlay, overlay_color, event.pos);
    }
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
  } else Q_UNREACHABLE();
  return ToolChanges::overlay;
}

namespace {

QRect polyBounds(const std::vector<QPoint> &polygon) {
  QRect rect;
  if (!polygon.empty()) {
    rect = {polygon.front(), QSize{1, 1}};
  }
  for (const QPoint p : polygon) {
    if (p.x() < rect.left()) rect.setLeft(p.x());
    if (p.y() < rect.top()) rect.setTop(p.y());
    if (p.x() > rect.right()) rect.setRight(p.x());
    if (p.y() > rect.bottom()) rect.setBottom(p.y());
  }
  return rect;
}

}

ToolChanges MaskSelectTool::mouseUp(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  if (mode == SelectMode::copy) {
    if (polygon.back() != event.pos) {
      polygon.push_back(event.pos);
    }
    const QRect bounds = polyBounds(polygon).intersected(source->image.data.rect());
    mask = QImage{bounds.size(), mask_format};
    clearImage(mask);
    drawFilledPolygon(mask, mask_color_on, polygon, -bounds.topLeft());
    selection = blitMaskImage(source->image.data, mask, bounds.topLeft());
    overlay = selection;
    colorToOverlay(overlay, mask);
    offset = bounds.topLeft() - event.pos;
    return ToolChanges::overlay;
  } else if (mode == SelectMode::paste) {
    blitImage(*event.overlay, overlay, event.pos + offset);
    return ToolChanges::overlay;
  } else Q_UNREACHABLE();
}

ToolChanges MaskSelectTool::keyPress(const ToolKeyEvent &event) {
  // @TODO make sure the mouse isn't down
  if (event.key == Qt::Key_P) {
    mode = opposite(mode);
  }
  statusMode(*event.status, mode);
  return ToolChanges::none;
}

void MaskSelectTool::setMode(const SelectMode newMode) {
  mode = newMode;
}

template <typename Derived>
DragPaintTool<Derived>::~DragPaintTool() {
  static_assert(std::is_base_of_v<DragPaintTool, Derived>);
}

template <typename Derived>
bool DragPaintTool<Derived>::attachCell(Cell *cell) {
  source = dynamic_cast<SourceCell *>(cell);
  if (source) {
    if (!compatible(cleanImage, source->image.data)) {
      cleanImage = makeCompatible(source->image.data);
    }
    return true;
  } else {
    return false;
  }
}

template <typename Derived>
void DragPaintTool<Derived>::detachCell() {
  assert(source);
  source = nullptr;
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseDown(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  startPos = event.pos;
  copyImage(cleanImage, source->image.data);
  color = selectColor(event.colors, event.button);
  return drawnChanges(that()->drawPoint(source->image, startPos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseMove(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  that()->drawOverlay(*event.overlay, event.pos);
  if (event.button == ButtonType::none) return ToolChanges::overlay;
  copyImage(source->image.data, cleanImage);
  return drawnChanges(that()->drawDrag(source->image, startPos, event.pos));
}

template <typename Derived>
ToolChanges DragPaintTool<Derived>::mouseUp(const ToolMouseEvent &event) {
  assert(source);
  clearImage(*event.overlay);
  copyImage(source->image.data, cleanImage);
  const bool drawn = that()->drawDrag(source->image, startPos, event.pos);
  startPos = no_point;
  return drawnChanges(drawn);
}

template <typename Derived>
bool DragPaintTool<Derived>::isDragging() const {
  return startPos != no_point;
}

template <typename Derived>
QRgb DragPaintTool<Derived>::getColor() const {
  return color;
}

template <typename Derived>
Derived *DragPaintTool<Derived>::that() {
  return static_cast<Derived *>(this);
}

LineTool::~LineTool() = default;

bool LineTool::drawPoint(Image &image, const QPoint pos) {
  return drawSquarePoint(image.data, getColor(), pos);
}

bool LineTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  return drawLine(image.data, getColor(), {start, end});
}

void LineTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, overlay_color, pos);
}

StrokedCircleTool::~StrokedCircleTool() = default;

void StrokedCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

int StrokedCircleTool::getRadius() const {
  return isDragging() ? radius : no_radius;
}

bool StrokedCircleTool::drawPoint(Image &, QPoint) {
  radius = 0;
  return false;
}

namespace {

double distance(const QPoint a, const QPoint b) {
  const int dx = a.x() - b.x();
  const int dy = a.y() - b.y();
  return std::sqrt(dx*dx + dy*dy);
}

int calcRadius(const QPoint start, const QPoint end) {
  return std::round(distance(start, end));
}

}

bool StrokedCircleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  radius = calcRadius(start, end);
  return drawStrokedEllipse(image.data, getColor(), circleToRect(start, radius, shape));
}

void StrokedCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, overlay_color, centerToRect(pos, shape));
}

FilledCircleTool::~FilledCircleTool() = default;

void FilledCircleTool::setShape(const CircleShape newShape) {
  shape = newShape;
}

int FilledCircleTool::getRadius() const {
  return isDragging() ? radius : no_radius;
}

bool FilledCircleTool::drawPoint(Image &, QPoint) {
  radius = 0;
  return false;
}

bool FilledCircleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  radius = calcRadius(start, end);
  return drawFilledEllipse(image.data, getColor(), circleToRect(start, radius, shape));
}

void FilledCircleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawFilledRect(overlay, overlay_color, centerToRect(pos, shape));
}

StrokedRectangleTool::~StrokedRectangleTool() = default;

QSize StrokedRectangleTool::getSize() const {
  return isDragging() ? size : no_size;
}

bool StrokedRectangleTool::drawPoint(Image &image, const QPoint pos) {
  size = QSize{1, 1};
  return drawSquarePoint(image.data, getColor(), pos);
}

bool StrokedRectangleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  size = rect.size();
  return drawStrokedRect(image.data, getColor(), rect);
}

void StrokedRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, overlay_color, pos);
}

FilledRectangleTool::~FilledRectangleTool() = default;

QSize FilledRectangleTool::getSize() const {
  return isDragging() ? size : no_size;
}

bool FilledRectangleTool::drawPoint(Image &image, const QPoint pos) {
  size = QSize{1, 1};
  return drawSquarePoint(image.data, getColor(), pos);
}

bool FilledRectangleTool::drawDrag(Image &image, const QPoint start, const QPoint end) {
  const QRect rect = QRect{start, end}.normalized();
  size = rect.size();
  return drawFilledRect(image.data, getColor(), rect);
}

void FilledRectangleTool::drawOverlay(QImage &overlay, const QPoint pos) {
  drawSquarePoint(overlay, overlay_color, pos);
}

bool TranslateTool::attachCell(Cell *cell) {
  if ((source = dynamic_cast<SourceCell *>(cell))) {
    if (!compatible(cleanImage, source->image.data)) {
      cleanImage = makeCompatible(source->image.data);
    }
    copyImage(cleanImage, source->image.data);
    pos = {0, 0};
    return true;
  } else if ((transform = dynamic_cast<TransformCell *>(cell))) {
    return true;
  }
  return false;
}

namespace {

template <typename A, typename B>
bool oneNotNull(A *a, B *b) {
  return !!a + !!b == 1;
}

}

void TranslateTool::detachCell() {
  source = nullptr;
  transform = nullptr;
}

ToolChanges TranslateTool::mouseDown(const ToolMouseEvent &event) {
  assert(oneNotNull(source, transform));
  if (event.button != ButtonType::primary) return ToolChanges::none;
  lastPos = event.pos;
  drag = true;
  return ToolChanges::none;
}

ToolChanges TranslateTool::mouseMove(const ToolMouseEvent &event) {
  assert(oneNotNull(source, transform));
  if (event.button != ButtonType::primary || !drag) return ToolChanges::none;
  translate(*event.status, event.pos - lastPos, event.colors.erase);
  lastPos = event.pos;
  return ToolChanges::cell;
}

ToolChanges TranslateTool::mouseUp(const ToolMouseEvent &event) {
  assert(oneNotNull(source, transform));
  if (event.button != ButtonType::primary || !drag) return ToolChanges::none;
  translate(*event.status, event.pos - lastPos, event.colors.erase);
  lastPos = event.pos;
  drag = false;
  return ToolChanges::cell;
}

namespace {

QPoint arrowToDir(const Qt::Key key) {
  switch (key) {
    case Qt::Key_Up: return {0, -1};
    case Qt::Key_Right: return {1, 0};
    case Qt::Key_Down: return {0, 1};
    case Qt::Key_Left: return {-1, 0};
    default: return {0, 0};
  }
}

}

ToolChanges TranslateTool::keyPress(const ToolKeyEvent &event) {
  assert(oneNotNull(source, transform));
  QPoint move = arrowToDir(event.key);
  if (move == QPoint{0, 0}) return ToolChanges::none;
  translate(*event.status, move, event.colors.erase);
  return ToolChanges::cell;
}

namespace {

Transform xformFromPos(const QPoint pos) {
  Transform xform;
  xform.posX = pos.x();
  xform.posY = pos.y();
  return xform;
}

}

void TranslateTool::translate(std::string &status, const QPoint move, const QRgb eraseColor) {
  assert(oneNotNull(source, transform));
  if (source) {
    pos += move;
    updateSourceImage(eraseColor);
    statusPos(status, pos);
  } else if (transform) {
    Transform &xform = transform->xform;
    xform.posX += move.x();
    xform.posY += move.y();
    statusPos(status, {xform.posX, xform.posY});
  } else Q_UNREACHABLE();
}

void TranslateTool::updateSourceImage(const QRgb eraseColor) {
  assert(source);
  assert(!transform);
  clearImage(source->image.data, eraseColor);
  Image src{cleanImage, xformFromPos(pos)};
  blitTransformedImage(source->image.data, src);
}

bool FlipTool::attachCell(Cell *cell) {
  if ((source = dynamic_cast<SourceCell *>(cell))) {
    return true;
  } else if ((transform = dynamic_cast<TransformCell *>(cell))) {
    return true;
  }
  return false;
}

void FlipTool::detachCell() {
  if (source) {
    updateSourceImage();
  }
}

namespace {

bool arrowToFlip(const Qt::Key key, Transform &xform) {
  switch (key) {
    // return true if changed
    case Qt::Key_Up:    return  std::exchange(xform.flipY, false);
    case Qt::Key_Right: return !std::exchange(xform.flipX, true);
    case Qt::Key_Down:  return !std::exchange(xform.flipY, true);
    case Qt::Key_Left:  return  std::exchange(xform.flipX, false);
    default: return false;
  }
}

Transform &getTransform(SourceCell *source, TransformCell *transform) {
  assert(oneNotNull(source, transform));
  if (source) {
    return source->image.xform;
  } else if (transform) {
    return transform->xform;
  } else Q_UNREACHABLE();
}

}

ToolChanges FlipTool::keyPress(const ToolKeyEvent &event) {
  assert(oneNotNull(source, transform));
  Transform &xform = getTransform(source, transform);
  if (arrowToFlip(event.key, xform)) {
    *event.status += "X: ";
    statusBool(*event.status, xform.flipX);
    *event.status += " Y: ";
    statusBool(*event.status, xform.flipY);
    return ToolChanges::cell;
  }
  return ToolChanges::none;
}

namespace {

void applyTransform(Image &image) {
  QImage temp = makeCompatible(image.data);
  blitTransformedImage(temp, image);
  copyImage(image.data, temp);
}

}

void FlipTool::updateSourceImage() {
  assert(source);
  assert(!transform);
  applyTransform(source->image);
  source->image.xform.flipX = false;
  source->image.xform.flipY = false;
}

bool RotateTool::attachCell(Cell *cell) {
  if ((source = dynamic_cast<SourceCell *>(cell))) {
    return true;
  } else if ((transform = dynamic_cast<TransformCell *>(cell))) {
    return true;
  }
  return false;
}

void RotateTool::detachCell() {
  if (source) {
    updateSourceImage();
  }
}

namespace {

quint8 arrowToRot(const Qt::Key key) {
  switch (key) {
    case Qt::Key_Up: return 3;
    case Qt::Key_Right: return 1;
    case Qt::Key_Down: return 1;
    case Qt::Key_Left: return 3;
    default: return 0;
  }
}

}

ToolChanges RotateTool::keyPress(const ToolKeyEvent &event) {
  assert(oneNotNull(source, transform));
  const quint8 rot = arrowToRot(event.key);
  if (rot) {
    quint8 &angle = getTransform(source, transform).angle;
    angle = (angle + rot) & 3;
    *event.status += "ANGLE: ";
    *event.status += std::to_string(angle * 90);
    return ToolChanges::cell;
  }
  return ToolChanges::none;
}

void RotateTool::updateSourceImage() {
  assert(source);
  assert(!transform);
  applyTransform(source->image);
  source->image.xform.angle = 0;
}
