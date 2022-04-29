#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "command_line.hpp"
#include "ttf_font.hpp"

struct Options {
  std::string font_file;
  int font_size;
  std::string output_directory;
  std::string text_file;
  std::string type;
  std::string title;
  bool status;
  int canvas_width;
  int canvas_height;
  int text_x;
  int text_y;
  int text_w;
  int text_h;
  int vspace;
  bool area_set;
  bool break_lines;
  bool info_mode;

  Options()
  {
    font_file = "VeraMono.ttf";
    font_size = 10;
    output_directory = "";
    type = "jpg";
      
    status = false;
      
    canvas_width  = 480;
    canvas_height = 272;
      
    vspace = 0;

    text_x = -1;
    text_y = -1;
    text_w = -1;
    text_h = -1;
    area_set = false;
    
    break_lines = false;
    info_mode   = false;
  }
};

Options options;


class TextStream
{
private:
  std::string text;
  int count;
  int lines;
public:
  TextStream(const std::string& text_)
    : text(text_),
      count(0),
      lines(0)
  {
  }
  
  char read() { 
    if (text[count] == '\n')
      lines += 1;

    return text[count++];
  }

  std::string read_word() {
    std::string word;

    if (text[count] == '\n' ||
        text[count] == ' '  ||
        text[count] == '\t')
      {
        word += text[count++];
        return word;
      }
    else
      {
        while (text[count] != '\n' &&
               text[count] != ' '  &&
               text[count] != '\t')
          {
            word  += text[count++];
          }
        return word;
      }
  }

  void unread(const std::string& word)
  {
    for(std::string::size_type i = 0; i < word.size(); ++i)
      {
        if (word[i] == '\n')
          {
            lines -= 1;
          }
        count -= 1;
      }
  }

  int get_line() { return lines; }
  
  bool eof() {
    return count >= int(text.size());
  }
};

class Renderer
{
public:
  int x;
  int y;
  TTFFont* font;
  Bitmap*  canvas;
  int start_x;
  int start_y;
  int text_width;
  int text_height;
  bool done;
  int chapter;
  TextStream* stream;

  Renderer()
  {
    x = 0;
    y = 0; // <- refers to the baseline of the font not to top-left corner!
    start_x = 0;
    start_y = 0;

    chapter = 0;
  }

  void print(const std::string& text)
  {
    for (std::string::const_iterator c = text.begin(); c != text.end(); ++c)
      {
        const TTFCharacter& glyph = font->get_character(*c);
        canvas->blit(*glyph.bitmap, 
                     x + glyph.x_offset + start_x,
                     y + glyph.y_offset + start_y);
        x += glyph.advance;
      }
  }

  void render(Bitmap& canvas_, TTFFont& font_, TextStream& stream_,
              int start_x_, int start_y_,
              int text_width_, int text_height_)
  {
    font   = &font_;
    canvas = &canvas_;
    start_x = start_x_;
    start_y = start_y_;
    text_width  = text_width_;
    text_height = text_height_;
    stream = &stream_;
    done = false;

    while(!stream->eof() && !done)
      {
        std::string word = stream->read_word();

        if (word == "\n")
          {
            if (x == 0 && y == 0)
              {
                // ignore newlines at the top of a file
              }
            else
              {
                x = 0;
                y += font->get_height() + options.vspace;

                if (y >= text_height)
                  {
                    // We are done with rendering
                    done = true;
                  }
              }
          }
        else if (word == "\t")
          { // magic to get taps correctly aligned 
            int cw = font->get_character(' ').advance * 8;
            x = (x/cw + 1) * cw;
          }
        else if (word == " ")
          { 
            print(" ");
          }
        else if (word == "###PAGEBREAK###")
          {
            //            print("                                                            * * *");
            chapter += 1;
            done = true;
          }
        else
          {
            // read a whole word, now try to render it
            if (x + font->get_width(word) >= text_width)
              { // word would print over-print, so we make a linebreak
                y += font->get_height() + options.vspace;
                x = 0;

                if (y >= text_height)
                  {
                    done = true;
                    stream->unread(word);
                  }
                else
                  {
                    print(word);
                  }
              }
            else
              {
                print(word);
              }
          }
      }
  }
};

void parse_args(int argc, char** argv, Options& options)
{
  CommandLine argp;

  argp.set_help_indent(24);
  argp.add_usage ("[OPTION]... TEXTFILE");
  argp.add_doc   ("text2image converts ascii text files to images");

  argp.add_group("Options:");
  argp.add_option('f', "font",      "FONT",    "Use FONT for rendering the text, must be a ttf font (default: VerdanaMono.ttf)");
  argp.add_option('s',  "size",     "NUM",     "Set the font size to NUM (default: 12)");
  argp.add_option('o', "output",    "DIR",    "Write the rendered image to DIR");
  argp.add_option('t',  "type",     "TYPE",    "Use TYPE format when writing the file, can be 'jpg' or 'pgm'");
  argp.add_option('l',  "status-line",   "",        "Add a statusbar to the rendering output");
  argp.add_option('a',  "area", "WxH+X+Y","limit text rendering to the given area");
  argp.add_option('n',  "title",    "NAME",    "Use NAME as title in the status line");
  argp.add_option('c',  "canvas",   "WxH",      "Use a canvas for size WxH+X+Y");
  argp.add_option('b',  "break",    "",        "Break long lines");
  argp.add_option('i',  "info",     "",        "Print information about the text");
  argp.add_option('v',  "vspace",   "NUM",     "Amount of pixels between lines");
  argp.add_option('h',  "help",     "",        "Display this help");
  
  argp.parse_args(argc, argv);

  while (argp.next())
    {
      switch (argp.get_key())
        {
        case 'f': // font
          options.font_file = argp.get_argument();
          break;

        case 'o': // output
          options.output_directory = argp.get_argument();
          break;

        case 't': // type
          if (argp.get_argument() != "jpg" || argp.get_argument() != "pgm")
            {
              std::cerr << "Only 'pgm' or 'jpg' allowed as format type options" << std::endl;
              exit(EXIT_FAILURE);              
            }
          options.type = argp.get_argument();
          break;

        case 'a': // area
          if (sscanf(argp.get_argument().c_str(), "%dx%d+%d+%d",
                     &options.text_w,
                     &options.text_h,
                     &options.text_x,
                     &options.text_y) != 4)
            {
              std::cerr << "Expected WIDTHxHEIGHT+X+Y as argument" << std::endl;
              exit(EXIT_FAILURE);
            }

          options.area_set = true;
          break;

        case 'c': // canvas
          if (sscanf(argp.get_argument().c_str(), "%dx%d", &options.canvas_width, &options.canvas_height) != 2)
            {
              std::cerr << "Expected WIDTHxHEIGHT as argument" << std::endl;
              exit(EXIT_FAILURE);
            }
          break;

        case 'i': // info
          options.info_mode = true;
          break;

        case 'n': // title
          options.title = argp.get_argument();
          break;

        case 'b': // break
          options.break_lines = true;
          break;

        case 'l': // status-line
          options.status = true;
          break;

        case 's': // size
          if (sscanf(argp.get_argument().c_str(), "%d", &options.font_size) != 1)
            {
              std::cerr << "Expected Integer as argument for --size" << std::endl;
              exit(EXIT_FAILURE);
            }
          break;

        case 'v': // vspace
          if (sscanf(argp.get_argument().c_str(), "%d", &options.vspace) != 1)
            {
              std::cerr << "Expected Integer as argument for --vspace" << std::endl;
              exit(EXIT_FAILURE);
            }
          break;


        case 'h':
          argp.print_help();
          exit(EXIT_SUCCESS);
          break;

        case CommandLine::REST_ARG:
          if (options.text_file.empty())
            {
              options.text_file = argp.get_argument();
            }
          else
            {
              std::cerr << "Only one text file can be given" << std::endl;
              exit(EXIT_FAILURE); 
            }
          break;

        default: 
          std::cerr << "Unknown argument: " << argp.get_key() << std::endl;
          exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv)
{
  parse_args(argc, argv, options);

  if (options.output_directory.empty())
  {
    std::cerr << "error: no --output argument given" << std::endl;
  }
  else if (options.text_file.empty())
    {
      std::cout << "Usage: " << argv[0] << " [OPTION]... TEXTFILE" << std::endl;
    }
  else
    {
      TTFFont::init();

      //std::cout << "Load font" << std::endl;
      TTFFont font(options.font_file, options.font_size);

      //  std::cout << "Create bitmap" << std::endl;
      Bitmap  bitmap(options.canvas_width, options.canvas_height);

      std::ifstream fin(options.text_file.c_str());
      std::istreambuf_iterator<char> first(fin), last;
      std::string text(first, last); 

      //std::cout << "Render..." << std::endl;

      if (!options.area_set)
        {
          options.text_x = 4;
          options.text_y = font.get_height();
          options.text_w = bitmap.get_width() - 4;
          options.text_h = bitmap.get_height() - font.get_height()*2;
        }

      
      TextStream stream(text);
      int page = 0;
  
      int chapter = 1;
      while(!stream.eof())
        { 
          Renderer renderer;
          renderer.render(bitmap, font, stream, options.text_x, options.text_y, options.text_w, options.text_h);

          page += 1;

          if (options.status)
            {
              std::ostringstream tstr;
              if (!options.title.empty())
                tstr << options.title << " - ";
          
              tstr << "Page "
                   << std::setfill(' ') << std::setw(2) << page << "/21 - " 
                   << std::setw(2) << 100 * page / 21 << "%";
              {
                TextStream tmpstr(tstr.str());
                Renderer renderer;
                renderer.render(bitmap, font, tmpstr,
                                bitmap.get_width()/2 - font.get_width(tstr.str())/2, 
                                bitmap.get_height() - font.get_height() + font.get_character('M').bitmap->get_height()+2,
                                bitmap.get_width(), bitmap.get_height());
              }
              bitmap.invert(0, bitmap.get_height() - font.get_height(),
                            bitmap.get_width(), bitmap.get_height());
            }

          std::ostringstream str;
          str << options.output_directory << "/";
          str << std::setfill('0') << std::setw(2) << chapter;
          str << "_"    << std::setfill('0') << std::setw(4) << page << ".jpg";

          std::cout << "Writing: " << str.str() << std::endl;
          bitmap.write_jpg(str.str());
          bitmap.clear();

          chapter += renderer.chapter;
        }
      TTFFont::deinit();
    }

  return 0;
}

/* EOF */
