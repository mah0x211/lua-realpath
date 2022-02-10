rockspec_format = '3.0'
package = 'realpath'
version = 'scm-1'
source = {
    url = 'git+https://github.com/mah0x211/lua-realpath.git',
}
description = {
    summary = 'canonicalize the pathname.',
    homepage = 'https://github.com/mah0x211/lua-realpath',
    license = 'MIT/X11',
    maintainer = 'Masatoshi Fukunaga'
}
dependencies = {
    'lua >= 5.1',
    'lauxhlib >= 0.1.0',
}
build = {
    type = 'builtin',
    modules = {
        realpath = {
            sources = { 'src/realpath.c' }
        },
    }
}
