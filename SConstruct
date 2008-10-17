import util, warnings, os, sys

# Ignore DeprecationWarnings to keep things readable (happens with scons 1.0.1 and Python 2.6)
warnings.simplefilter('ignore', DeprecationWarning)

platform = util.get_platform()
arch = util.get_arch()


# Set up generic environment
if platform == 'win32':
    env_generic = Environment(tools = ['mingw', 'nasm'])
    # Require user to have a properly set up PATH so scons can find gcc/nasm
    env_generic['ENV']['PATH'] = os.environ.get('PATH')
else:
    # Assume gcc etc are available but print a warning if the platform is unrecognised
    env_generic = Environment(tools = ['gcc', 'gnulink', 'nasm'])
    if platform not in ['linux', 'freebsd', 'darwin']:
        print 'WARNING: Unrecognised platform (' + platform + ')'


# Pretty printing
env_generic['CCCOMSTR'] = 'Compiling $SOURCE -> $TARGET'
env_generic['ASCOMSTR'] = 'Assembling $SOURCE -> $TARGET'
env_generic['LINKCOMSTR'] = 'Linking $TARGET'


# OS-specific setup
if platform == 'win32':
    env_generic.Append(CPPPATH = [r'C:\MinGW\devil\include',
                                  r'C:\MinGW\gtkstuff\include\atk-1.0',
                                  r'C:\MinGW\gtkstuff\include\cairo',
                                  r'C:\MinGW\gtkstuff\include\pango-1.0',
                                  r'C:\MinGW\gtkstuff\include\glib-2.0',
                                  r'C:\MinGW\gtkstuff\lib\glib-2.0\include',
                                  r'C:\MinGW\gtkstuff\include\gtk-2.0',
                                  r'C:\MinGW\gtkstuff\lib\gtk-2.0\include',
                                  r'C:\MinGW\gtkstuff\include\libglade-2.0'])
    env_generic.Append(LIBPATH = [r'C:\MinGW\devil\lib', 
                                  r'C:\MinGW\gtkstuff\lib'])
    env_generic.Append(LIBS = ['devil', 'ilu', 'glade-2.0', 'glib-2.0', 'gtk-win32-2.0',
                               'gobject-2.0', 'gdk-win32-2.0'])

    # GTK+ on Windows needs this
    env_generic.Append(CCFLAGS = ['-mms-bitfields'])
    
    if arch == 'x86':
        env_generic.Append(ASFLAGS = ['-f win32'])
    elif arch == 'x86_64':
        env_generic.Append(ASFLAGS = ['-f win64'])

else:
    env_generic.ParseConfig("pkg-config gtk+-2.0 --libs --cflags")
    env_generic.ParseConfig("pkg-config libglade-2.0 --libs --cflags")
    env_generic.ParseConfig("pkg-config gthread-2.0 --libs --cflags")
    # libglade needs this to connect UI callbacks
    env_generic.Append(LINKFLAGS = ['-rdynamic'])
    env_generic.Append(LIBS = ['IL', 'ILU'])

    if arch == 'x86':
        env_generic.Append(ASFLAGS = ['-f elf32'])
    elif arch == 'x86_64':
        env_generic.Append(ASFLAGS = ['-f elf64'])

#env_generic.Append(CCFLAGS = ['-pg', '-fno-inline']) # Profiling
env_generic.Append(CCFLAGS = ['-Wall', '-std=c99'])
#env_generic.Append(CPPDEFINES = ['SKIP_GTK']) # Skip drawing in GTK (benchmarking)

# Only enable assembly on x86
if arch not in ['x86', 'x86_64']:
    env_generic.Append(CPPDEFINES = ['NO_ASM'])


# Customize for release build
env_release = env_generic.Clone()
env_release.Append(CCFLAGS = ['-O2', '-fomit-frame-pointer', '-ffast-math'])
# TODO: allow setting march through commandline options to scons?
#env_release.Append(CCFLAGS = ['-march=core2'])


# Customize for debug build
env_debug = env_generic.Clone()
env_debug.Append(CCFLAGS = ['-g'])
env_debug.Append(ASFLAGS = ['-g'])
env_debug.Append(CPPDEFINES = ['DEBUG'])


env = env_release
SConscript('src/SConscript', exports='env arch', variant_dir='build/release', duplicate=0)

env = env_debug
SConscript('src/SConscript', exports='env arch', variant_dir='build/debug', duplicate=0)

