//
//  gradient tool.cpp
//  Animera
//
//  Created by Indi Kernick on 6/10/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "gradient tool.hpp"

#include "painting.hpp"

void LinearGradientTool::detachCell() {
  ctx->clearStatus();
}

void LinearGradientTool::mouseLeave(const ToolLeaveEvent &) {
  clearImage(*ctx->overlay);
  ctx->emitChanges(ToolChanges::overlay);
  ctx->clearStatus();
}

void LinearGradientTool::mouseDown(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  
  if (event.button == ButtonType::secondary) {
    mode = opposite(mode);
  }
  
  updateStatus(event, {event.pos, QSize{1, 1}});
  
  if (event.button != ButtonType::primary) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  
  startPos = event.pos;
  ctx->requireCell(toRect(startPos));
  cleanCell = *ctx->cell;
  const QPoint pos = ctx->cell->pos;
  drawSquarePoint(ctx->cell->img, ctx->colors.primary, startPos - pos);
  ctx->emitChanges(ToolChanges::cell_overlay);
}

void LinearGradientTool::mouseMove(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  
  QRect rect = unite(startPos, event.pos);
  updateStatus(event, rect);
  
  if (event.button != ButtonType::primary) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  
  *ctx->cell = cleanCell;
  ctx->growCell(rect);
  drawGradient(rect, event.pos);
}

void LinearGradientTool::mouseUp(const ToolMouseEvent &event) {
  clearImage(*ctx->overlay);
  drawSquarePoint(*ctx->overlay, tool_overlay_color, event.pos);
  if (event.button != ButtonType::primary) {
    return ctx->emitChanges(ToolChanges::overlay);
  }
  *ctx->cell = cleanCell;
  QRect rect = unite(startPos, event.pos);
  ctx->growCell(rect);
  drawGradient(rect, event.pos);
  ctx->finishChange();
}

void LinearGradientTool::updateStatus(const ToolMouseEvent &event, const QRect rect) {
  StatusMsg status;
  status.appendLabeled(mode);
  if (event.button != ButtonType::primary) {
    status.appendLabeled(event.pos);
  } else {
    status.appendLabeled(rect);
  }
  ctx->showStatus(status);
}

void LinearGradientTool::drawGradient(QRect rect, const QPoint endPos) {
  QImage &img = ctx->cell->img;
  const QPoint pos = ctx->cell->pos;
  QRgb first = ctx->colors.primary;
  QRgb second = ctx->colors.secondary;
  if (mode == LineGradMode::hori) {
    if (startPos.x() > endPos.x()) {
      std::swap(first, second);
    }
    rect = rect.translated(-pos);
    ctx->emitChanges(drawHoriGradient(img, first, second, rect));
  } else if (mode == LineGradMode::vert) {
    if (startPos.y() > endPos.y()) {
      std::swap(first, second);
    }
    rect = rect.translated(-pos);
    ctx->emitChanges(drawVertGradient(img, first, second, rect));
  } else Q_UNREACHABLE();
}