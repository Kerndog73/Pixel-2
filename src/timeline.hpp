//
//  timeline.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 6/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_hpp
#define timeline_hpp

#include "cell.hpp"

class Timeline final : public QObject {
  Q_OBJECT

public:
  Timeline();

  void initDefault();
  void serialize(QIODevice *) const;
  void deserialize(QIODevice *);

public Q_SLOTS:
  void initCanvas(Format, QSize);

  void nextFrame();
  void prevFrame();
  void layerBelow();
  void layerAbove();
  
  void beginSelection();
  void continueSelection();
  void endSelection();
  void clearSelection();
  
  void insertLayer();
  void removeLayer();
  void moveLayerUp();
  void moveLayerDown();
  
  void insertFrame();
  void insertNullFrame();
  void removeFrame();
  
  void clearCell();
  void extendCell();
  void requestCell();
  
  void setCurrPos(CellPos);
  void setVisibility(LayerIdx, bool);
  void setName(LayerIdx, std::string_view);

Q_SIGNALS:
  void currPosChanged(CellPos);
  void currCellChanged(Cell *);
  void selectionChanged(CellRect);
  
  void visibilityChanged(LayerIdx, bool);
  void nameChanged(LayerIdx, std::string_view);
  
  void frameChanged(const Frame &);
  void layerChanged(LayerIdx, const Spans &);
  
  void frameCountChanged(FrameIdx);
  void layerCountChanged(LayerIdx);
  
private:
  Layers layers;
  CellPos currPos;
  CellRect selection;
  FrameIdx frameCount;
  QSize canvasSize;
  Format canvasFormat;
  
  CellPtr makeCell() const;
  Cell *getCell(CellPos);
  Frame getFrame(FrameIdx);
  LayerIdx layerCount() const;
  
  void changePos();
  void changeFrame();
  void changeSpan(LayerIdx);
  void changeLayers(LayerIdx, LayerIdx);
  void changeFrameCount();
  void changeLayerCount();
};

#endif