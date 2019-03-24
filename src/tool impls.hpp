//
//  tool impls.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tool_impls_hpp
#define tool_impls_hpp

#include "tool.hpp"
#include "paint params.hpp"

class SourceCell;
class TransformCell;

class BrushTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
  ToolChanges mouseDown(const ToolMouseEvent &) override;
  ToolChanges mouseMove(const ToolMouseEvent &) override;
  ToolChanges mouseUp(const ToolMouseEvent &) override;

  void setWidth(int);
  void setMode(SymmetryMode);

private:
  QPoint lastPos = no_point;
  SourceCell *source = nullptr;
  // @TODO restore these to 1 and none
  int width = 8;
  SymmetryMode mode = SymmetryMode::both;
  QRgb color = 0;
  
  void symPoint(std::string &, QPoint);
  bool symPoint(QImage &, QRgb, QPoint);
  bool symLine(QImage &, QRgb, QLine);
};

class FloodFillTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
  ToolChanges mouseDown(const ToolMouseEvent &) override;
  ToolChanges mouseMove(const ToolMouseEvent &) override;

private:
  SourceCell *source = nullptr;
};

class RectangleSelectTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
  ToolChanges mouseDown(const ToolMouseEvent &) override;
  ToolChanges mouseMove(const ToolMouseEvent &) override;
  ToolChanges mouseUp(const ToolMouseEvent &) override;
  ToolChanges keyPress(const ToolKeyEvent &) override;

  void setMode(SelectMode);
  
private:
  QPoint startPos = no_point;
  SourceCell *source = nullptr;
  QImage selection;
  QImage overlay;
  QPoint offset;
  SelectMode mode = SelectMode::copy;
};

class MaskSelectTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
  ToolChanges mouseDown(const ToolMouseEvent &) override;
  ToolChanges mouseMove(const ToolMouseEvent &) override;
  ToolChanges mouseUp(const ToolMouseEvent &) override;
  ToolChanges keyPress(const ToolKeyEvent &) override;
  
  void setMode(SelectMode);

private:
  std::vector<QPoint> polygon;
  SourceCell *source = nullptr;
  QImage selection;
  QImage mask;
  QImage overlay;
  QPoint offset;
  SelectMode mode = SelectMode::copy;
};

template <typename Derived>
class DragPaintTool : public Tool {
public:
  ~DragPaintTool();
  
  bool attachCell(Cell *) override final;
  void detachCell() override final;
  ToolChanges mouseDown(const ToolMouseEvent &) override final;
  ToolChanges mouseMove(const ToolMouseEvent &) override final;
  ToolChanges mouseUp(const ToolMouseEvent &) override final;

protected:
  bool isDragging() const;
  QRgb getColor() const;

private:
  QPoint startPos = no_point;
  SourceCell *source = nullptr;
  QImage cleanImage;
  QRgb color = 0;
  
  Derived *that();
};

class LineTool final : public DragPaintTool<LineTool> {
public:
  friend class DragPaintTool;

  ~LineTool();

private:
  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class StrokedCircleTool final : public DragPaintTool<StrokedCircleTool> {
public:
  friend class DragPaintTool;

  ~StrokedCircleTool();

  void setShape(CircleShape);
  int getRadius() const;
  
private:
  CircleShape shape = CircleShape::c1x1;
  int radius = no_radius;

  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class FilledCircleTool final : public DragPaintTool<FilledCircleTool> {
public:
  friend class DragPaintTool;
  
  ~FilledCircleTool();
  
  void setShape(CircleShape);
  int getRadius() const;

private:
  CircleShape shape = CircleShape::c1x1;
  int radius = no_radius;
  
  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class StrokedRectangleTool final : public DragPaintTool<StrokedRectangleTool> {
public:
  friend class DragPaintTool;
  
  ~StrokedRectangleTool();
  
  QSize getSize() const;

private:
  QSize size = no_size;
  
  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class FilledRectangleTool final : public DragPaintTool<FilledRectangleTool> {
public:
  friend class DragPaintTool;
  
  ~FilledRectangleTool();

  QSize getSize() const;

private:
  QSize size = no_size;
  
  bool drawPoint(Image &, QPoint);
  bool drawDrag(Image &, QPoint, QPoint);
  void drawOverlay(QImage &, QPoint);
};

class TranslateTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
  ToolChanges mouseDown(const ToolMouseEvent &) override;
  ToolChanges mouseMove(const ToolMouseEvent &) override;
  ToolChanges mouseUp(const ToolMouseEvent &) override;
  ToolChanges keyPress(const ToolKeyEvent &) override;

private:
  SourceCell *source = nullptr;
  TransformCell *transform = nullptr;
  QImage cleanImage;
  QPoint lastPos = no_point;
  QPoint pos = no_point;
  bool drag = false;
  
  void translate(std::string &, QPoint, QRgb);
  void updateSourceImage(QRgb);
};

class FlipTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
  ToolChanges keyPress(const ToolKeyEvent &) override;
  
private:
  SourceCell *source = nullptr;
  TransformCell *transform = nullptr;
  
  void updateSourceImage();
};

class RotateTool final : public Tool {
public:
  bool attachCell(Cell *) override;
  void detachCell() override;
  ToolChanges keyPress(const ToolKeyEvent &) override;

private:
  SourceCell *source = nullptr;
  TransformCell *transform = nullptr;
  
  void updateSourceImage();
};

#endif
