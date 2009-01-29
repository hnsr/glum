import util, warnings

# Ignore DeprecationWarnings to keep things readable (happens with scons 1.0.1 and Python 2.6)
warnings.simplefilter('ignore', DeprecationWarning)

platform = util.get_platform()
arch = util.get_arch()

# Build variable stuff
vars = Variables('build.conf')
vars.Add('MARCH', 'Sets the -march gcc optimization flag', '')
vars.Add('PROFILING', 'If set to 1, adds profiling information to both debug/release executables',0)
vars.Add('SKIP_GTK', 'If set to 1, skips drawing framebuffer to GTK drawingarea for benchmarking'\
    ' purposes', 0)
vars.Add('NO_ASM', 'If set to 1, no assembly files will be built', 0)
vars.Add('VERBOSE', 'If set to 1, the full compilation commands will be shown',0)
vars.Add('WIN_GTK_PATH', 'Should be set to the path where the GTK+ headers/libs reside on Windows',
    'C:\\MinGW\\gtkstuff')
vars.Add('WIN_DEVIL_PATH', 'Should be set to the path where the DevIL headers/libs reside on '\
    'Windows', 'C:\\MinGW\\devil')
vars.Add('WIN_PATH', 'Semicolon-seperated paths in which scons looks for the gcc, nasm executables',
    'C:\\MinGW\\bin;C:\\Program Files\\nasm')


# Set up generic environment
if platform == 'win32':
    env_generic = Environment(variables = vars, tools = ['mingw', 'nasm'])

    env_generic['ENV']['PATH'] = env_generic['WIN_PATH']
    gtk_path = env_generic['WIN_GTK_PATH']
    devil_path = env_generic['WIN_DEVIL_PATH']
    env_generic.Append(CPPPATH = [devil_path + '\include',
                                  gtk_path + '\\include\\atk-1.0',
                                  gtk_path + '\\include\\cairo',
                                  gtk_path + '\\include\\pango-1.0',
                                  gtk_path + '\\include\\glib-2.0',
                                  gtk_path + '\\lib\\glib-2.0\\include',
                                  gtk_path + '\\include\\gtk-2.0',
                                  gtk_path + '\\lib\\gtk-2.0\\include',
                                  gtk_path + '\\include\\libglade-2.0'])
    env_generic.Append(LIBPATH = [devil_path + '\\lib', 
                                  gtk_path + '\\lib'])
    env_generic.Append(LIBS = ['devil', 'ilu', 'glade-2.0', 'glib-2.0', 'gtk-win32-2.0',
                               'gobject-2.0', 'gdk-win32-2.0'])

    # GTK+ on Windows needs this
    env_generic.Append(CCFLAGS = ['-mms-bitfields'])

    if arch == 'x86':
        env_generic.Append(ASFLAGS = ['-f win32'])
    elif arch == 'x86_64':
        env_generic.Append(ASFLAGS = ['-f win64'])

else:
    # Assume gcc etc are available but print a warning if the platform is unrecognised
    if platform not in ['linux', 'freebsd', 'darwin']:
        print 'WARNING: Unrecognised platform (' + platform + ')'

    env_generic = Environment(variables = vars, tools = ['gcc', 'gnulink', 'nasm'])

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

env_generic.Append(CCFLAGS = ['-Wall', '-std=c99'])

if not util.is_defined(env_generic['VERBOSE']):
    env_generic['CCCOMSTR'] = 'Compiling $SOURCE -> $TARGET'
    env_generic['ASCOMSTR'] = 'Assembling $SOURCE -> $TARGET'
    env_generic['LINKCOMSTR'] = 'Linking $TARGET'

if util.is_defined(env_generic['PROFILING']):
    env_generic.Append(CCFLAGS = ['-pg', '-fno-inline'])
    env_generic.Append(LINKFLAGS = ['-pg'])

# Skip copying image to GTK+ drawingarea (for benchmarking) if requested
if util.is_defined(env_generic['SKIP_GTK']):
    env_generic.Append(CPPDEFINES = ['SKIP_GTK'])

# Only enable assembly on x86 and x86_64 and if NO_ASM wasn't defined
if arch not in ['x86', 'x86_64'] or util.is_defined(env_generic['NO_ASM']):
    env_generic.Append(CPPDEFINES = ['NO_ASM'])


Help(vars.GenerateHelpText(env_generic))


# Customize for release build
env_release = env_generic.Clone()
# Don't add omit-frame-pointer when profiling
if util.is_defined(env_generic['PROFILING']):
    env_release.Append(CCFLAGS = ['-O2', '-ffast-math'])
else:
    env_release.Append(CCFLAGS = ['-O2', '-fomit-frame-pointer', '-ffast-math'])

env_release.Append(CCFLAGS = ['-march=$MARCH'] if str(env_release['MARCH']) else [] )


# Customize for debug build
env_debug = env_generic.Clone()
env_debug.Append(CCFLAGS = ['-g'])
env_debug.Append(ASFLAGS = ['-g'])
env_debug.Append(CPPDEFINES = ['DEBUG'])


env = env_release
SConscript('src/SConscript', exports='env arch', variant_dir='build/release', duplicate=0)

env = env_debug
SConscript('src/SConscript', exports='env arch', variant_dir='build/debug', duplicate=0)

