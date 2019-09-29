//
//  palette widget.cpp
//  Animera
//
//  Created by Indi Kernick on 3/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "palette widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include "color handle.hpp"
#include <Graphics/format.hpp>
#include "widget painting.hpp"
#include "radio button widget.hpp"
#include <QtWidgets/qgridlayout.h>

class PaletteColorWidget final : public RadioButtonWidget, public ColorHandle {
  Q_OBJECT
  
public:
  PaletteColorWidget(QWidget *parent, QRgb &color, const int index, const Format format)
    : RadioButtonWidget{parent}, color{color}, index{index}, format{format} {
    setFixedSize(pal_tile_size, pal_tile_size);
    CONNECT(this, toggled, this, attachColor);
    loadPixmaps();
  }

Q_SIGNALS:
  void shouldAttachColor(ColorHandle *);
  void shouldSetColor(QRgb);
  void shouldSetIndex(int);
  void paletteColorChanged();

private Q_SLOTS:
  void attachColor() {
    Q_EMIT shouldAttachColor(this);
  }

private:
  QRgb &color;
  int index;
  Format format;
  QPixmap selectBlack;
  QPixmap selectWhite;

  void loadPixmaps() {
    // TODO: cache
    QBitmap bitmap{":/Color Picker/palette select.pbm"};
    bitmap = bitmap.scaled(bitmap.size() * glob_scale);
    selectBlack = bakeColoredBitmap(bitmap, QColor{0, 0, 0});
    selectWhite = bakeColoredBitmap(bitmap, QColor{255, 255, 255});
  }

  void paintChecker(QPainter &painter) {
    constexpr int bord = glob_border_width;
    constexpr int half = (pal_tile_size - bord) / 2;
    painter.setBrush(QColor{edit_checker_a});
    painter.drawRect(bord, bord, half, half);
    painter.drawRect(bord + half, bord + half, half, half);
    painter.setBrush(QColor{edit_checker_b});
    painter.drawRect(bord + half, bord, half, half);
    painter.drawRect(bord, bord + half, half, half);
  }
  
  static QColor toQColor(const gfx::Color color) {
    return QColor{color.r, color.g, color.b, color.a};
  }
  
  gfx::Color getGColor() const {
    switch (format) {
      case Format::index:
      case Format::rgba:
        return gfx::ARGB::color(color);
      case Format::gray:
        return gfx::YA::color(color);
    }
  }
  
  bool useWhiteSelect() const {
    const gfx::Color gColor = getGColor();
    return gfx::gray(gColor) < 128 && gColor.a >= 128;
  }
  
  void paintColor(QPainter &painter) {
    painter.setBrush(toQColor(getGColor()));
    painter.drawRect(
      glob_border_width, glob_border_width,
      pal_tile_size - glob_border_width, pal_tile_size - glob_border_width
    );
  }
  
  void paintBorder(QPainter &painter) {
    painter.setBrush(glob_border_color);
    painter.drawRect(0, 0, pal_tile_size, glob_border_width);
    painter.drawRect(0, glob_border_width, glob_border_width, pal_tile_size);
  }
  
  void paintSelect(QPainter &painter) {
    if (isChecked()) {
      if (useWhiteSelect()) {
        painter.drawPixmap(glob_border_width, glob_border_width, selectWhite);
      } else {
        painter.drawPixmap(glob_border_width, glob_border_width, selectBlack);
      }
    }
  }

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.setPen(Qt::NoPen);
    paintChecker(painter);
    paintColor(painter);
    paintBorder(painter);
    paintSelect(painter);
  }
  
  void mousePressEvent(QMouseEvent *event) override {
    RadioButtonWidget::mousePressEvent(event);
    if (event->button() == Qt::RightButton) {
      if (format == Format::index) {
        Q_EMIT shouldSetIndex(index);
        click();
      } else {
        Q_EMIT shouldSetColor(color);
      }
    }
  }
  
  QRgb getInitialColor() const override {
    return color;
  }
  void changeColor(const QRgb newColor) override {
    color = newColor;
    repaint();
    Q_EMIT paletteColorChanged();
  }
  QString getName() const override {
    return "Palette " + QString::number(index);
  }
  void detach() override {
    uncheck();
  }
};

class PaletteTableWidget final : public QWidget {
  Q_OBJECT
  
public:
  PaletteTableWidget(QWidget *parent, const Format format)
    : QWidget{parent}, format{format} {}

public Q_SLOTS:
  void setPalette(const PaletteSpan palette) {
    createWidgets(palette);
    setupLayout();
    connectSignals();
  }
  void attachIndex(const int index) {
    colors[index]->click();
  }
  void updatePalette() {
    for (PaletteColorWidget *colorWidget : colors) {
      if (colorWidget->isChecked()) {
        Q_EMIT shouldAttachColor(nullptr);
        colorWidget->click();
        break;
      }
    }
    repaint();
    Q_EMIT paletteColorChanged();
  }

Q_SIGNALS:
  void shouldAttachColor(ColorHandle *);
  void shouldSetColor(QRgb);
  void shouldSetIndex(int);
  void paletteChanged(PaletteSpan);
  void paletteColorChanged();

private:
  std::vector<PaletteColorWidget *> colors;
  Format format;
  
  void createWidgets(const PaletteSpan palette) {
    colors.reserve(pal_colors);
    for (int i = 0; i != pal_colors; ++i) {
      colors.push_back(new PaletteColorWidget{this, palette[i], i, format});
    }
  }
  
  void setupLayout() {
    auto *grid = new QGridLayout{this};
    grid->setSpacing(0);
    grid->setContentsMargins(0, 0, 0, 0);
    
    for (int y = 0; y != pal_height; ++y) {
      for (int x = 0; x != pal_width; ++x) {
        grid->addWidget(colors[y * pal_width + x], y, x);
      }
    }
    
    setFixedSize(
      pal_width * pal_tile_size + glob_border_width,
      pal_height * pal_tile_size + glob_border_width
    );
  }
  
  void connectSignals() {
    for (PaletteColorWidget *colorWidget : colors) {
      CONNECT(colorWidget, shouldAttachColor,   this, shouldAttachColor);
      CONNECT(colorWidget, shouldSetColor,      this, shouldSetColor);
      CONNECT(colorWidget, shouldSetIndex,      this, shouldSetIndex);
      CONNECT(colorWidget, paletteColorChanged, this, paletteColorChanged);
    }
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.setPen(Qt::NoPen);
    painter.setBrush(glob_border_color);
    painter.drawRect(width() - glob_border_width, 0, glob_border_width, height());
    painter.drawRect(0, height() - glob_border_width, width() - glob_border_width, glob_border_width);
  }
};

PaletteWidget::PaletteWidget(QWidget *parent)
  : QScrollArea{parent} {
  setFrameShape(NoFrame);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  setFixedWidth(pick_svgraph_rect.widget().width() + 2 * glob_border_width);
  setMinimumHeight(pal_tile_size + glob_border_width);
  setAlignment(Qt::AlignHCenter);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PaletteWidget::initCanvas(const Format format) {
  table = new PaletteTableWidget{this, format};
  setWidget(table);
  CONNECT(table, shouldAttachColor,   this, shouldAttachColor);
  CONNECT(table, shouldSetColor,      this, shouldSetColor);
  CONNECT(table, shouldSetIndex,      this, shouldSetIndex);
  CONNECT(table, paletteColorChanged, this, paletteColorChanged);
}

void PaletteWidget::setPalette(PaletteSpan palette) {
  table->setPalette(palette);
}

void PaletteWidget::attachIndex(const int index) {
  table->attachIndex(index);
}

void PaletteWidget::updatePalette() {
  table->updatePalette();
}

#include "palette widget.moc"
