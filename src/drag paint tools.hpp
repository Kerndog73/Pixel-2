﻿//
//  drag paint tools.hpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_drag_paint_tools_hpp
#define animera_drag_paint_tools_hpp

#include "tool.hpp"
#include "cel.hpp"

template <typename Derived>
class DragPaintTool : public Tool {
public:
  ~DragPaintTool();
  
  void attachCelImage() override final;
  void detachCelImage() override final;
  void mouseLeave(const ToolLeaveEvent &) override final;
  void mouseDown(const ToolMouseDownEvent &) override final;
  void mouseMove(const ToolMouseMoveEvent &) override final;
  void mouseUp(const ToolMouseUpEvent &) override final;

protected:
  PixelVar getColor() const;

private:
  QPoint startPos = no_point;
  CelImage cleanImage;
  PixelVar color;
  
  Derived *that();
};

class LineTool final : public DragPaintTool<LineTool> {
public:
  template <typename>
  friend class DragPaintTool;

  ~LineTool();
  
  void setRadius(int);

private:
  int radius = line_radius.def;
 
  bool drawPoint(QImage &, PixelVar, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class StrokedCircleTool final : public DragPaintTool<StrokedCircleTool> {
public:
  template <typename>
  friend class DragPaintTool;

  ~StrokedCircleTool();

  void setShape(gfx::CircleShape);
  void setThick(int);
  
private:
  gfx::CircleShape shape = gfx::CircleShape::c1x1;
  int thickness = circ_thick.def;

  bool drawPoint(QImage &, PixelVar, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class FilledCircleTool final : public DragPaintTool<FilledCircleTool> {
public:
  template <typename>
  friend class DragPaintTool;
  
  ~FilledCircleTool();
  
  void setShape(gfx::CircleShape);

private:
  gfx::CircleShape shape = gfx::CircleShape::c1x1;
  
  bool drawPoint(QImage &, PixelVar, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class StrokedRectangleTool final : public DragPaintTool<StrokedRectangleTool> {
public:
  template <typename>
  friend class DragPaintTool;
  
  ~StrokedRectangleTool();
  
  void setThick(int);

private:
  int thickness = rect_thick.def;

  bool drawPoint(QImage &, PixelVar, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

class FilledRectangleTool final : public DragPaintTool<FilledRectangleTool> {
public:
  template <typename>
  friend class DragPaintTool;
  
  ~FilledRectangleTool();

private:
  bool drawPoint(QImage &, PixelVar, QPoint);
  bool drawDrag(QImage &, QPoint, QPoint);
  void updateStatus(StatusMsg &, QPoint, QPoint);
  QRect pointRect(QPoint);
  QRect dragRect(QPoint, QPoint);
};

#endif
