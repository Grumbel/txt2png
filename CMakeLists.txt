cmake_minimum_required(VERSION 4.0)
project(txt2png VERSION 0.0.0)

include(GNUInstallDirs)

find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)

add_executable(txt2png
  src/bitmap.cpp
  src/command_line.cpp
  src/command_line_generic.cpp
  src/ttf_font.cpp
  src/txt2png.cpp)

target_link_libraries(txt2png PUBLIC
  Freetype::Freetype
  JPEG::JPEG)

install(TARGETS txt2png
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# EOF #
