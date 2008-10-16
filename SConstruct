import platform, warnings, os

# Ignore DeprecationWarnings to keep things readable (happens with scons 1.0.1 and Python 2.6)
warnings.simplefilter('ignore', DeprecationWarning)

system = platform.system()
machine = platform.machine()
(bits, linkage) = platform.architecture()

# Try to figure out machine architecture
if machine in ['x86', 'i386', 'i686']:
    machine = 'x86'
elif machine in ['x86_64']:
    pass
# Sometimes Python on Windows returns an empty string, so as a last resort, just assume it's either
# x86 or x86_64 and use platform.architecture() to figure out which it is...
elif system == 'Windows':
    if bits == '32bit':
        machine = 'x86'
    elif bits == '64bit':
        machine = 'x86_64'
    else:
        print 'ERROR: platform.architecture() returned something unexpected (' + bits + \
              '), fix arch detection'
        Exit(1)
else:
    print 'WARNING: Unknown machine arch'
    machine = 'unknown'

print 'System: ' + system + '  Machine: ' + machine

# Set up generic environment
if system == 'Windows':
    env_generic = Environment(tools = ['mingw', 'nasm'])
    # Require user to have a properly set up PATH so scons can find gcc/nasm
    env_generic['ENV']['PATH'] = os.environ.get('PATH')
elif system == 'Linux':
    # TODO: remove default, keep tools to the minimum needed
    env_generic = Environment(tools = ['default', 'nasm'])
else:
    print 'ERROR: Unsupported operating system'
    Exit(1)


# Pretty printing
env_generic['CCCOMSTR'] = 'Compiling $SOURCE -> $TARGET'
env_generic['ASCOMSTR'] = 'Assembling $SOURCE -> $TARGET'
env_generic['LINKCOMSTR'] = 'Linking $TARGET'

# OS-specific setup
if system == 'Windows':
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
    env_generic.Append(CCFLAGS = ['-mms-bitfields']) # Needed by gtk+ on Windows
    
    if machine == 'x86':
        env_generic.Append(ASFLAGS = ['-f win32'])
    elif machine == 'x86_64':
        env_generic.Append(ASFLAGS = ['-f win64'])

elif system == 'Linux':
    env_generic.ParseConfig("pkg-config gtk+-2.0 --libs --cflags")
    env_generic.ParseConfig("pkg-config libglade-2.0 --libs --cflags")
    env_generic.ParseConfig("pkg-config gthread-2.0 --libs --cflags")
    env_generic.Append(LINKFLAGS = ['-rdynamic']) # Needed by libglade
    env_generic.Append(LIBS = ['IL', 'ILU'])
    if machine == 'x86':
        env_generic.Append(ASFLAGS = ['-f elf32'])
    elif machine == 'x86_64':
        env_generic.Append(ASFLAGS = ['-f elf64'])


#env_generic.Append(CCFLAGS = ['-pg', '-fno-inline']) # Profiling
env_generic.Append(CCFLAGS = ['-Wall', '-std=c99'])
env_generic.Append(LIBS = ['m'])                # Additional libs
#env_generic.Append(CPPDEFINES = ['SKIP_GTK'])  # Skip drawing in GTK (benchmarking)

# Only enable assembly on x86
env_generic['CPPDEFINES'] = []
if machine != 'x86':
    env_generic.Append(CPPDEFINES = ['NO_ASM'])



# Customize for release build
env_release = env_generic.Clone()
env_release.Append(CCFLAGS = ['-O2', '-fomit-frame-pointer', '-ffast-math'])
# TODO: allow setting march through commandline options to scons?
#env_release.Append(CCFLAGS = ['-march=nocona'])


# Customize for debug build
env_debug = env_generic.Clone()
env_debug.Append(CCFLAGS = ['-g'])
env_debug.Append(ASFLAGS = ['-g'])
env_debug.Append(CPPDEFINES = ['DEBUG'])


env = env_release
SConscript('src/SConscript', exports='env', variant_dir='build/release', duplicate=0)

env = env_debug
SConscript('src/SConscript', exports='env', variant_dir='build/debug', duplicate=0)

