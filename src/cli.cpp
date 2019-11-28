//
//  cli.cpp
//  Animera
//
//  Created by Indi Kernick on 23/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cli.hpp"

#include "sprite.hpp"
#include "strings.hpp"
#include "application.hpp"
#include <QtCore/qtextstream.h>

CLI::CLI(int &argc, char **argv)
  : argc{argc}, argv{argv} {}

namespace {

// We can't use help_text because help_text is too fancy
// This is still a million times better than QCommandLineParser though!

const char doc_text[] =
R"(Usage:
  Animera [--help]
  Animera open <file>
  Animera export [--name=<pattern> --directory=<path>]
                 [--layer-stride=<integer> --layer-offset=<integer>]
                 [--frame-stride=<integer> --frame-offset=<integer>]
                 [--no-composite --layer=<range> --frame=<range>]
                 [--format=<format>]
                 [--scale-x=<integer> --scale-y=<integer> --scale=<integer>]
                 [--angle=<integer>] <file>

Options:
  -n, --name <pattern>  Desc.
  -d, --directory <path>  Desc.
  --layer-stride <integer>  Desc.
  --layer-offset <integer>  Desc.
  --frame-stride <integer>  Desc.
  --frame-offset <integer>  Desc.
  -c, --no-composite  Desc.
  -l, --layer <range>  Desc.
  -f, --frame <range>  Desc.
  -F, --format <format>  Desc.
  --scale-x <integer>  Desc.
  --scale-y <integer>  Desc.
  --scale <integer>  Desc.
  -a, --angle <integer>  Desc.
  -h, --help  Desc.
)";

// TODO: Long help and short help

const char help_text[] =
R"(Usage:
    Animera [--help]
    Animera open <file>
    Animera export [--name --directory]
                   [--layer-stride --layer-offset --frame-stride --frame-offset]
                   [--no-composite --layer --frame --format]
                   [(--scale-x --scale-y) | --scale] [--angle] <file>

Options:
    -n, --name <pattern>
        Name pattern for the sprite. By default this is "sprite_%000F".
        This pattern may contain format sequences.
        The available format sequences are:
            %F  frame number
            %L  layer number
            %%  percent sign
        Zeros may appear after the % character to pad the number with zeros.
        Examples assuming that "e.animera" contains 2 layers and 2 frames:
            Animera export -n "a_%F" e.animera        a_0 a_1
            Animera export -n "b%%_%000F" e.animera   b%_000 b%_001
            Animera export -c -n "c_%L_%F" e.animera  c_0_0 c_0_1 c_1_0 c_1_1
    
    -d, --directory <path>
        Directory to write files to. By default this is ".". The output file
        paths are generated by concatenating this and the above options:
            <path> "/" <pattern> ".png"
        Paths ending with a / are accepted.
        
    --layer-stride <integer>
        This is similar to the --frame-stride option but for layers.
        
    --layer-offset <integer>
        This is similar to the --frame-offset option but for layers.
    
    --frame-stride <integer>
        A stride that is multiplied by the frame number before evaluating the
        name pattern. The stride cannot be 0 but can be negative.
        Examples assuming that "e.animera" contains 2 frames:
            Animera export -n "a_%F" --frame-stride 2 e.animera   a_0 a_2
            Animera export -n "b_%F" --frame-stride -1 e.animera  b_0 b_-1
        This option is useful when combined with the --frame-offset option.
    
    --frame-offset <integer>
        An offset that is added to the frame number before evaluating the name
        pattern. Examples assuming that "e.animera" contains 2 frames:
            Animera export -n "a_%F" --frame-offset 2 e.animera   a_2 a_3
            Animera export -n "b_%F" --frame-offset -1 e.animera  b_-1 b_0
    
    -c, --no-composite
        By default, the cells that make up a frame are composited so that each
        frame is exported as a single image. If this option is present, layers
        are not composited and the cells that make up a frame are exported
        individually. The presence or absence of this option can affect the
        output formats available.
    
    -l, --layer <range>
        This is similar to the --frame option but for layers.
    
    -f, --frame <range>
        The range of frames to export. By default, all frames are exported. This
        may be a single frame such as 0 or an inclusive range such as 1..3.
        Either or both sides of the range may be omitted.
        Examples assuming a sprite with 4 frames:
            1..2            frames 1 and 2
            ..2   or  0..2  frames 0, 1 and 2
            1..   or  1..3  frames 1, 2 and 3
            ..    or  0..3  frames 0, 1, 2 and 3
            1     or  1..1  frame 1
    
    -F, --format <format>
        The resulting output files are always PNGs. This option corresponds to
        the color type of the PNGs. The output formats available depend on the
        format of the input sprite. Given the input format, these are the valid
        output formats (the first in each list is the default):
            RGBA
                rgba        (8-bit RGBA)
            Indexed (with --no-composite)
                index       (8-bit Indexed)
                gray        (8-bit Grayscale)
                monochrome  (1-bit Grayscale)
            Indexed (without --no-composite)
                rgba        (8-bit RGBA)
            Grayscale
                gray-alpha  (8-bit Grayscale with alpha)
                gray        (8-bit Grayscale)
                monochrome  (1-bit Grayscale)
    
    --scale-x <integer>
        The horizontal scale factor applied to each output image. The scale
        factor cannot be 0 but can be negative, in this case, the image is
        flipped horizontally. If this option is present, the --scale option
        cannot be.
    
    --scale-y <integer>
        The vertical scale factor applied to each output image. The scale factor
        cannot be 0 but can be negative, in this case, the image is flipped
        vertically. If this option is present, the --scale option cannot be.
    
    -s, --scale <integer>
        The scale factor applied to each output image. This option is equivalent
        to using the --scale-x and --scale-y options with the same scale factor.
        If this option is present, neither --scale-x nor --scale-y can be.
    
    -a, --angle <integer>
        The angle of rotation applied to each output image. The image can only
        be rotated in 90 degree increments. This means that --angle 1 will
        rotate clockwise by 90 degrees. The angle doesn't need to be within
        [0, 4]. This means that --angle -3, --angle 1 and --angle 5 are all
        equivalent. The rotation is applied after the scale is applied.
    
    -h, --help
        Displays this help.)";
        
QTextStream &console() {
  static QTextStream stream{stdout};
  return stream;
}

}

#include <iostream>

int CLI::exec() {
  std::map<std::string, docopt::value> flags;
  try {
    flags = docopt::docopt_parse(doc_text, {argv + 1, argv + argc}, true);
  } catch (docopt::DocoptExitHelp &) {
    console() << help_text;
    return 0;
  } catch (docopt::DocoptArgumentError &e) {
    console() << "Command line error\n";
    console() << e.what();
    return 1;
  }
  
  for (const auto &flag : flags) {
    std::cout << flag.first << " = " << flag.second << '\n';
  }
  
  if (flags.at("open").asBool()) {
    return execOpen(flags);
  } else if (flags.at("export").asBool()) {
    return execExport(flags);
  } else {
    return execDefault();
  }
}

int CLI::execDefault() const {
  Application app{argc, argv};
  app.waitForOpenEvent();
  return app.exec();
}

int CLI::execOpen(const std::map<std::string, docopt::value> &flags) const {
  Application app{argc, argv};
  app.openFile(toLatinString(flags.at("<file>").asString()));
  return app.exec();
}

int CLI::execExport(const std::map<std::string, docopt::value> &flags) const {
  QCoreApplication app{argc, argv};
  Sprite sprite;
  if (Error err = sprite.openFile(toLatinString(flags.at("<file>").asString())); err) {
    console() << "File open error\n";
    console() << err.msg() << '\n';
    return 1;
  }
  ExportOptions options;
  ExportSpriteInfo info = {
    sprite.timeline.getLayers(), sprite.timeline.getFrames(), {}, {}
  };
  const Format format = sprite.getFormat();
  initDefaultOptions(options, format);
  if (Error err = readExportOptions(options, info, format, flags); err) {
    console() << "Configuration error\n";
    console() << err.msg() << '\n';
    return 1;
  }
  sprite.timeline.setCurrPos({info.selection.minL, info.selection.minF});
  sprite.timeline.beginSelection();
  sprite.timeline.setCurrPos({info.selection.maxL, info.selection.maxF});
  sprite.timeline.endSelection();
  sprite.timeline.setCurrPos(info.current);
  if (Error err = sprite.exportSprite(options); err) {
    console() << "Export error\n";
    console() << err.msg() << '\n';
    return 1;
  }
  return 0;
}
