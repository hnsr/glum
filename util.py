import sys, os, platform as plat, re


# Return sanitized platform name (i.e. 'linux', 'win32', 'darwin' etc.)
def get_platform():

    platform = sys.platform

    # Special case win32/os2, for all other platforms ending with a number, remove it since it's the
    # major version number appended. This should be safe I think.. based on the listing at
    # http://docs.python.org/library/sys.html#sys.platform)
    if platform not in ['win32', 'os2']:
        platform = re.sub('[0-9]*\Z', '', platform)

    return platform


# Figure out machine architecture, this is kind of hacky since platform.machine() is inconsistent
# and isn't guaranteed to return anything (not on Windows at least). For now, returns either 'x86',
# 'x86_64' or 'unknown'.
def get_arch():

    machine = plat.machine()
    platform = get_platform()

    if machine in ['x86', 'i386', 'i686']:
        arch = 'x86'

    elif machine in ['x86_64', 'amd64']:
        arch = 'x86_64'

    # Sometimes Python on Windows returns an empty string, so as a last resort, just assume it's
    # either x86 or x86_64 and use platform.architecture() to figure out which it is...
    elif machine == '' and platform == 'win32':
        
        (bits, linkage) = platform.architecture()

        if bits == '32bit':
            arch = 'x86'

        elif bits == '64bit':
            arch = 'x86_64'
    else:
        arch = 'unknown'

    return arch


