/**
 *  Copyright (C) 2022 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

// lua
#include "lua_realpath.h"
// std
#include <limits.h>

static size_t REALPATH_BUFSIZ = PATH_MAX;
static char *REALPATH_BUF     = NULL;

static int realpath_lua(lua_State *L)
{
    size_t len       = 0;
    const char *path = lauxh_checklstring(L, 1, &len);
    int normalize    = lauxh_optboolean(L, 2, 0);
    int resolve      = lauxh_optboolean(L, 3, 1);

    lua_settop(L, 1);
    // perform normalization only
    if (!resolve) {
        return lua_realpath_normalize(L, (char *)path, len);
    }

    // perform normalization before resolving the path
    if (normalize) {
        int rv = lua_realpath_normalize(L, (char *)path, len);
        if (rv != 1) {
            return rv;
        }
        path = lua_tostring(L, -1);
    }

    // perform path resolution
    path = realpath(path, REALPATH_BUF);
    lua_settop(L, 0);
    if (path) {
        lua_pushstring(L, path);
        return 1;
    }

    // got error
    lua_pushnil(L);
    lua_errno_new(L, errno, "realpath");
    return 2;
}

LUALIB_API int luaopen_realpath(lua_State *L)
{
    long pathmax = pathconf(".", _PC_PATH_MAX);

    lua_errno_loadlib(L);

    // set the maximum number of bytes in a pathname
    if (pathmax != -1) {
        REALPATH_BUFSIZ = pathmax;
    }
    // allocate the buffer for realpath
    REALPATH_BUF = lua_newuserdata(L, REALPATH_BUFSIZ);
    // holds until the state closes
    luaL_ref(L, LUA_REGISTRYINDEX);

    lua_pushcfunction(L, realpath_lua);

    return 1;
}
