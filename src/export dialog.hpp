//
//  export dialog.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef export_dialog_hpp
#define export_dialog_hpp

#include "image.hpp"
#include <QtWidgets/qdialog.h>

class ComboBoxWidget;
class TextInputWidget;
class NumberInputWidget;
class TextPushButtonWidget;

enum class LayerSelect {
  all_composited,
  all,
  current
};

enum class FrameSelect {
  all,
  current
};

enum class ExportFormat {
  rgba,
  indexed,
  grayscale,
  monochrome
};

struct Line {
  int stride;
  int offset;
};

constexpr int apply(const Line line, const int x) {
  return x * line.stride + line.offset;
}

struct ExportOptions {
  QString name;
  QString directory;
  Line layerLine;
  Line frameLine;
  LayerSelect layerSelect;
  FrameSelect frameSelect;
};

class ExportDialog final : public QDialog {
  Q_OBJECT
  
public:
  ExportDialog(QWidget *, Format);
  
Q_SIGNALS:
  void exportSprite(ExportOptions);

private Q_SLOTS:
  void finalize();
  void updateFormatOptions(int);

private:
  Format format;
  ExportOptions options;
  TextInputWidget *name;
  NumberInputWidget *layerStride;
  NumberInputWidget *layerOffset;
  NumberInputWidget *frameStride;
  NumberInputWidget *frameOffset;
  ComboBoxWidget *layerSelect;
  ComboBoxWidget *frameSelect;
  ComboBoxWidget *formatSelect;
  
  TextPushButtonWidget *ok;
  TextPushButtonWidget *cancel;
  
  void addFormatOptions();
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif