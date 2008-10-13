
# Set up global environment
env_generic = Environment(tools = ['default', 'nasm'])

# Pretty printing
env_generic['CCCOMSTR'] = 'Compiling $SOURCE -> $TARGET'
env_generic['ASCOMSTR'] = 'Assembling $SOURCE -> $TARGET'
env_generic['LINKCOMSTR'] = 'Linking $TARGET'

# Setup LIBS/CCFLAGS for gtk/glade/gthread
env_generic.ParseConfig("pkg-config gtk+-2.0 --libs --cflags")
env_generic.ParseConfig("pkg-config libglade-2.0 --libs --cflags")
env_generic.ParseConfig("pkg-config gthread-2.0 --libs --cflags")

env_generic.Append(CCFLAGS = ['-Wall', '-std=c99'])
#env_generic.Append(CCFLAGS = ['-pg', '-fno-inline']) # Profiling
env_generic.Append(ASFLAGS = ['-f elf']) # TODO: some magic should be used here (detecting current
                                         # arch for setting right format, or disable assembly
                                         # completely if unknown arch)
env_generic.Append(LIBS = ['m', 'IL', 'ILU'])         # Additional libs
env_generic.Append(LINKFLAGS = ['-rdynamic'])         # Needed by libglade
env_generic.Append(CPPDEFINES = ['NO_ASM'])           # Disable assembly
#env_generic.Append(CPPDEFINES = ['SKIP_GTK'])        # Skip drawing in GTK (benchmarking)


# Customize for release build
env_release = env_generic.Clone()
env_release.Append(CCFLAGS = ['-O2', '-fomit-frame-pointer', '-ffast-math'])
# TODO: allow setting march through commandline options to scons?
env_release.Append(CCFLAGS = ['-march=nocona'])


# Customize for debug build
env_debug = env_generic.Clone()
env_debug.Append(CCFLAGS = ['-g'])
env_debug.Append(ASFLAGS = ['-g'])
env_debug.Append(CPPDEFINES = ['DEBUG'])


env = env_release
SConscript('src/SConscript', exports='env', variant_dir='build/release', duplicate=0)

env = env_debug
SConscript('src/SConscript', exports='env', variant_dir='build/debug', duplicate=0)

