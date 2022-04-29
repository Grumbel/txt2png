# -*- python -*-

env = Environment()
env.ParseConfig('freetype-config --libs --cflags')
env['LIBS'] += ['jpeg']
env['CXXFLAGS'] += ['-g', '-O0', '-Wall']
env.Program('txt2png', ['txt2png.cpp',
                        'bitmap.cpp',
                        'ttf_font.cpp',
                        'command_line.cpp',
                        'command_line_generic.cpp'
                        ])

# EOF #
