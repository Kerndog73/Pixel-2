﻿//
//  cpp export backend.cpp
//  Animera
//
//  Created by Indiana Kernick on 18/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "cpp export backend.hpp"

#include "strings.hpp"
#include "file io.hpp"
#include "composite.hpp"
#include <QtCore/qdir.h>
#include "export png.hpp"
#include <QtCore/qmath.h>
#include <QtCore/qtextstream.h>

namespace {

constexpr char sprite_id_operators[] = R"(
[[nodiscard]] constexpr ANIMERA_SPRITE_RECT getSpriteRect(const SpriteID id) noexcept {
  assert(0 <= static_cast<int>(id));
  assert(static_cast<int>(id) < sprite_count);
  return sprite_rects[static_cast<int>(id)];
}

[[nodiscard]] constexpr SpriteID operator+(SpriteID id, const int off) noexcept {
  assert(0 <= static_cast<int>(id));
  assert(static_cast<int>(id) < sprite_count);
  id = SpriteID{static_cast<int>(id) + off};
  assert(0 <= static_cast<int>(id));
  assert(static_cast<int>(id) < sprite_count);
  return id;
}

[[nodiscard]] constexpr SpriteID operator-(const SpriteID id, const int off) noexcept {
  return id + -off;
}

[[nodiscard]] constexpr int operator-(const SpriteID a, const SpriteID b) noexcept {
  assert(0 <= static_cast<int>(a));
  assert(static_cast<int>(a) <= sprite_count);
  assert(0 <= static_cast<int>(b));
  assert(static_cast<int>(b) < sprite_count);
  return static_cast<int>(a) - static_cast<int>(b);
}

constexpr SpriteID &operator+=(SpriteID &id, const int off) noexcept {
  id = id + off;
  return id;
}

constexpr SpriteID &operator-=(SpriteID &id, const int off) noexcept {
  id = id - off;
  return id;
}

constexpr SpriteID &operator++(SpriteID &id) noexcept {
  return id += 1;
}

constexpr SpriteID &operator--(SpriteID &id) noexcept {
  return id -= 1;
}

constexpr SpriteID operator++(SpriteID &id, int) noexcept {
  const SpriteID copy = id;
  id += 1;
  return copy;
}

constexpr SpriteID operator--(SpriteID &id, int) noexcept {
  const SpriteID copy = id;
  id -= 1;
  return copy;
}
)";

constexpr char sprite_rect_def[] = R"(
#ifndef ANIMERA_SPRITE_RECT
#define ANIMERA_SPRITE_RECT ::animera::SpriteRect
struct SpriteRect {
  int x = -1, y = -1;
  int w = 0, h = 0;
};
#endif

#ifndef ANIMERA_MAKE_SPRITE_RECT
#define ANIMERA_MAKE_SPRITE_RECT(X, Y, W, H) ANIMERA_SPRITE_RECT{X, Y, W, H}
#endif
)";

void convertToIdentifier(QString &str) {
  for (QChar &ch : str) {
    if (!ch.isLetterOrNumber()) {
      ch = '_';
    }
  }
  if (str.isEmpty() || str.front().isDigit()) {
    str.prepend('_');
  }
}

}

Error CppExportBackend::initAtlas(PixelFormat format, const QString &name, const QString &dir) {
  if (format == PixelFormat::index) {
    return "C++ Export Backend does not support indexed pixel format";
  }
  
  packer.init(format);
  enumeration.clear();
  appendEnumerator("null_", ~std::size_t{});
  array.clear();
  names.clear();
  names.insert("null_");
  collision.clear();
  atlasDir = dir;
  atlasName = name.isEmpty() ? "atlas" : name;
  return {};
}

void CppExportBackend::addName(
  const std::size_t i,
  const ExportNameParams &params,
  const ExportNameState &state
) {
  QString name = params.name;
  int baseName = name.size();
  appendLayerName(name, params, state);
  int layerName = name.size();
  appendGroupName(name, params, state);
  int groupName = name.size();
  appendFrameName(name, params, state);
  int frameName = name.size();
  
  {
    // sometimes an underscore is prepended
    const int sizeBefore = name.size();
    convertToIdentifier(name);
    if (name.size() == sizeBefore + 1) {
      ++baseName;
      ++layerName;
      ++groupName;
      ++frameName;
    }
  }
  
  const bool hasLayerName = layerName > baseName;
  const bool hasGroupName = groupName > layerName;
  const bool hasFrameName = frameName > groupName;
  
  if (hasFrameName && state.frame == state.groupBegin) {
    if (hasGroupName && state.group == GroupIdx{0}) {
      if (hasLayerName && state.layer == LayerIdx{0}) {
        addAlias(name.left(baseName), "beg_", i);
      }
      addAlias(name.left(layerName), "beg_", i);
    }
    addAlias(name.left(groupName), "beg_", i);
  }
  
  appendEnumerator(name, i);
  insertName(name);
  
  if (hasFrameName && state.frame - state.groupBegin == state.frameCount - FrameIdx{1}) {
    addAlias(name.left(groupName), "end_", i + 1);
    if (hasGroupName && state.group == state.groupCount - GroupIdx{1}) {
      addAlias(name.left(layerName), "end_", i + 1);
      if (hasLayerName && state.layer == state.layerCount - LayerIdx{1}) {
        addAlias(name.left(baseName), "end_", i + 1);
      }
    }
  }
}

void CppExportBackend::addSizes(const std::size_t count, const QSize size) {
  packer.append(count, size);
}

void CppExportBackend::addWhiteName() {
  appendEnumerator("whitepixel_", packer.count());
  insertName("whitepixel_");
  packer.appendWhite();
}

QString CppExportBackend::hasNameCollision() {
  return collision;
}

Error CppExportBackend::packRectangles() {
  return packer.pack();
}

Error CppExportBackend::initAnimation(const Format format, PaletteCSpan) {
  if (format == Format::index) {
    return "C++ Export Backend does not support indexed animation format";
  }
  return {};
}

Error CppExportBackend::addImage(const std::size_t i, const QImage &img) {
  appendRectangle(packer.copy(i, img));
  return {};
}

Error CppExportBackend::addWhiteImage() {
  appendRectangle(packer.copyWhite(packer.count() - 1));
  return {};
}

Error CppExportBackend::finalize() {
  TRY(writeCpp());
  return writeHpp();
}

void CppExportBackend::addAlias(QString base, const char *alias, const std::size_t value) {
  if (!base.isEmpty()) base += '_';
  base += alias;
  appendEnumerator(base, value);
  insertName(base);
}

void CppExportBackend::appendEnumerator(const QString &name, const std::size_t value) {
  enumeration += "  ";
  enumeration += name;
  enumeration += " = ";
  if (value == ~std::size_t{}) {
    enumeration += "-1";
  } else {
    enumeration += QString::number(value);
  }
  enumeration += ",\n";
}

void CppExportBackend::appendRectangle(const QRect &rect) {
  array += "  ANIMERA_MAKE_SPRITE_RECT(";
  array += QString::number(rect.x());
  array += ", ";
  array += QString::number(rect.y());
  array += ", ";
  array += QString::number(rect.width());
  array += ", ";
  array += QString::number(rect.height());
  array += "),\n";
}

void CppExportBackend::insertName(const QString &name) {
  if (collision.isEmpty() && !names.insert(name).second) {
    collision = name;
  }
}

Error CppExportBackend::writeBytes(QIODevice &dev, const char *data, const std::size_t size) {
  constexpr int bytes_per_line = (80 - 2) / 6;
  constexpr char hex_chars[] = "0123456789ABCDEF";
  char hex[] = "0x00, ";

  for (std::size_t i = 0; i != size; ++i) {
    if (i % bytes_per_line == 0) {
      if (dev.write("\n  ", 3) != 3) {
        return dev.errorString();
      }
    }
    
    static_assert(CHAR_BIT == 8);
    const unsigned char byte = data[i];
    hex[2] = hex_chars[byte >> 4];
    hex[3] = hex_chars[byte & 15];
    if (dev.write(hex, 6) != 6) {
      return dev.errorString();
    }
  }
  
  return {};
}

Error CppExportBackend::writeCpp() {
  QBuffer textureBuffer;
  textureBuffer.open(QIODevice::ReadWrite);
  TRY(packer.writePng(textureBuffer));
  
  QString nameSpace = atlasName;
  convertToIdentifier(nameSpace);
  
  FileWriter writer;
  TRY(writer.open(atlasDir + QDir::separator() + atlasName + ".cpp"));
  writer.dev().setTextModeEnabled(true);
  QTextStream stream{&writer.dev()};
  stream << "// This file was generated by Animera\n";
  stream << '\n';
  stream << "#include <cstddef>\n";
  stream << '\n';
  stream << "namespace animera {\n";
  stream << '\n';
  stream << "inline namespace " << nameSpace << " {\n";
  stream << '\n';
  stream << "const std::size_t texture_size = " << textureBuffer.size() << ";\n";
  stream << '\n';
  stream << "const unsigned char texture_data[] = {";
  stream.flush();
  
  QByteArray &textureArray = textureBuffer.buffer();
  TRY(writeBytes(writer.dev(), textureArray.data(), static_cast<std::size_t>(textureArray.size())));
  stream << '\n';
  stream << "};\n";
  stream << '\n';
  stream << "}\n";
  stream << '\n';
  stream << "}\n";
  stream.flush();
  
  if (stream.status() == QTextStream::WriteFailed) {
    return "Error writing to file";
  }
  
  return writer.flush();
}

Error CppExportBackend::writeHpp() {
  FileWriter writer;
  TRY(writer.open(atlasDir + QDir::separator() + atlasName + ".hpp"));
  writer.dev().setTextModeEnabled(true);
  QTextStream stream{&writer.dev()};

  QString nameSpace = atlasName;
  convertToIdentifier(nameSpace);
  QString headerGuard = nameSpace.toUpper();

  stream << "// This file was generated by Animera\n";
  stream << '\n';
  stream << "#ifndef ANIMERA_" << headerGuard << "_HPP\n";
  stream << "#define ANIMERA_" << headerGuard << "_HPP\n";
  stream << '\n';
  stream << "#include <cassert>\n";
  stream << "#include <cstddef>\n";
  stream << '\n';
  stream << "namespace animera {\n";
  stream << sprite_rect_def;
  stream << '\n';
  stream << "inline namespace " << nameSpace << " {\n";
  stream << '\n';
  stream << "constexpr int sprite_count = " << packer.count() << ";\n";
  stream << "constexpr int texture_width = " << packer.width() << ";\n";
  stream << "constexpr int texture_height = " << packer.height() << ";\n";
  stream << "extern const std::size_t texture_size;\n";
  stream << "extern const unsigned char texture_data[];\n";
  stream << '\n';
  stream << "enum class SpriteID {\n";
  stream << enumeration;
  stream << "};\n";
  stream << '\n';
  stream << "constexpr ANIMERA_SPRITE_RECT sprite_rects[sprite_count] = {\n";
  stream << array;
  stream << "};\n";
  stream << sprite_id_operators;
  stream << '\n';
  stream << "}\n";
  stream << '\n';
  stream << "}\n";
  stream << '\n';
  stream << "#endif\n";
  
  stream.flush();
  
  if (stream.status() == QTextStream::WriteFailed) {
    return "Error writing to file";
  }
  
  return writer.flush();
}