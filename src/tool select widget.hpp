﻿//
//  tool select widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_tool_select_widget_hpp
#define animera_tool_select_widget_hpp

#include "status msg.hpp"
#include "current tool.hpp"
#include <QtWidgets/qscrollarea.h>

class ToolWidget;
class QHBoxLayout;
class ToolParamWidget;
class ToolParamBarWidget;

class ToolSelectWidget final : public QScrollArea {
  Q_OBJECT

  friend class ToolWidget;

public:
  ToolSelectWidget(QWidget *, ToolParamBarWidget *);

Q_SIGNALS:
  void celImageModified(QRect);
  void overlayModified(QRect);
  void shouldShowNorm(std::string_view);
  void changingAction();
  void shouldGrowCelImage(QRect);
  void shouldShrinkCelImage(QRect);
  void shouldLock();
  void shouldUnlock();

public Q_SLOTS:
  void mouseEnter(QPoint);
  void mouseLeave();
  void mouseDown(QPoint, ButtonType);
  void mouseMove(QPoint);
  void mouseUp(QPoint, ButtonType);
  void keyPress(Qt::Key);
  
  void setOverlay(QImage *);
  void setCelImage(CelImage *);
  void setColors(ToolColors);
  void setPalette(PaletteCSpan);
  void initCanvas(Format, QSize);
  void resizeCanvas(QSize);
  
private Q_SLOTS:
  void setTool(ToolWidget *, Tool *);
  void lockTool();
  void unlockTool();
  
private:
  ToolParamBarWidget *bar = nullptr;
  QWidget *box = nullptr;
  std::vector<ToolWidget *> tools;
  CurrentTool currTool;
  ToolWidget *currWidget = nullptr;
  ToolCtx ctx;
  bool mouseIn = false;
  
  void ensureVisible(ToolWidget *);
  template <typename ParamWidget>
  void pushToolWidget(QHBoxLayout *);
  void createTools();
  void setupLayout();
  void connectSignals();
};

#endif
