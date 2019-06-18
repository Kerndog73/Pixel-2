//
//  porter duff.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef porter_duff_hpp
#define porter_duff_hpp

#include "color.hpp"
#include "surface.hpp"

struct Factor {
  uint8_t a, b;
};

// Porter-Duff compositing with straight alpha
// https://keithp.com/~keithp/porterduff/p253-porter.pdf

// Using floats makes this a tiny bit faster

inline Color porterDuff(const Color a, const Color b, const Factor f) {
  const uint32_t cA = a.a*f.a + b.a*f.b;
  if (cA == 0) {
    return {0, 0, 0, 0};
  } else {
    const uint8_t cR = (a.a*f.a*a.r + b.a*f.b*b.r) / cA;
    const uint8_t cG = (a.a*f.a*a.g + b.a*f.b*b.g) / cA;
    const uint8_t cB = (a.a*f.a*a.b + b.a*f.b*b.b) / cA;
    return {cR, cG, cB, static_cast<uint8_t>(cA / 255)};
  }
}

// Using a lambda to encourage inlining
#define MODE(NAME, FACTOR_A, FACTOR_B)                                          \
  constexpr auto mode_##NAME = [](                                              \
    [[maybe_unused]] const uint8_t a,                                           \
    [[maybe_unused]] const uint8_t b                                            \
  ) noexcept {                                                                  \
    return Factor{                                                              \
      static_cast<uint8_t>(FACTOR_A),                                           \
      static_cast<uint8_t>(FACTOR_B)                                            \
    };                                                                          \
  }

#define MODE_ALIAS(NAME, ALIAS, FACTOR_A, FACTOR_B)                             \
  MODE(NAME, FACTOR_A, FACTOR_B);                                               \
  constexpr auto mode_##ALIAS = mode_##NAME

MODE(      clear,              0,       0);
MODE_ALIAS(a,        src,      255,     0);
MODE_ALIAS(b,        dst,      0,       255);
MODE_ALIAS(a_over_b, src_over, 255,     255 - a);
MODE_ALIAS(b_over_a, dst_over, 255 - b, 255);
MODE_ALIAS(a_in_b,   src_in,   b,       0);
MODE_ALIAS(b_in_a,   dst_in,   0,       a);
MODE_ALIAS(a_out_b,  src_out,  255 - b, 0);
MODE_ALIAS(b_out_a,  dst_out,  0, 255 - a);
MODE_ALIAS(a_atop_b, src_atop, b, 255 - a);
MODE_ALIAS(b_atop_a, dst_atop, 255 - b, a);
MODE(      xor,                255 - b, 255 - a);

#undef MODE_ALIAS
#undef MODE

template <typename Mode>
Color porterDuff(const Color a, const Color b, const Mode mode) {
  return porterDuff(a, b, mode(a.a, b.a));
}

/*

struct Format {
  static Color toColor(Pixel);
  static Pixel toPixel(Color);
};

*/

template <typename Format, typename Pixel, typename Mode>
void porterDuffRegion(
  const Mode mode,
  const Surface<Pixel> dst,
  const CSurface<Pixel> src,
  const QPoint srcPos
) {
  const QRect srcRect = {srcPos, src.size()};
  const QRect dstRect = srcRect.intersected(dst.rect());
  if (dstRect.isEmpty()) return;
  
  auto srcRowIter = src.range({dstRect.topLeft() - srcPos, dstRect.size()}).begin();
  for (auto row : dst.range(dstRect)) {
    const Pixel *srcPixelIter = (*srcRowIter).begin();
    for (Pixel &pixel : row) {
      pixel = Format::toPixel(porterDuff(
        Format::toColor(*srcPixelIter),
        Format::toColor(pixel),
        mode
      ));
      ++srcPixelIter;
    }
    ++srcRowIter;
  }
}

template <typename Format, typename Pixel, typename Mode>
void porterDuff(
  const Mode mode,
  const Surface<Pixel> dst,
  const CSurface<Pixel> src
) {
  Q_ASSUME(dst.size() == src.size());
  porterDuffRegion<Format>(mode, dst, src, {0, 0});
}

#endif
