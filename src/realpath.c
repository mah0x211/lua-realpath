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
#include <stdlib.h>

#define REALPATH_GC_MT "realpath.gc"

static inline void update_gcvar(void **gcvar, void *ref)
{
    if (*gcvar) {
        free(*gcvar);
    }
    *gcvar = ref;
}

// __gc metamethod for the GC guard userdata
static int gc_lua(lua_State *L)
{
    update_gcvar((void **)lua_touserdata(L, 1), NULL);
    return 0;
}

static inline void **new_gcvar(lua_State *L)
{
    void **gcvar = (void **)lua_newuserdata(L, sizeof(void *));
    *gcvar       = NULL;
    lauxh_setmetatable(L, REALPATH_GC_MT);
    return gcvar;
}

static inline void init_gcmt(lua_State *L)
{
    // register the GC guard metatable used for the NULL buf case
    if (luaL_newmetatable(L, REALPATH_GC_MT)) {
        lua_pushcfunction(L, gc_lua);
        lua_setfield(L, -2, "__gc");
    }
    lua_pop(L, 1);
}

static int realpath_lua(lua_State *L)
{
    size_t len         = 0;
    const char *path   = lauxh_checklstring(L, 1, &len);
    int normalize      = lauxh_optboolean(L, 2, 0);
    int resolve        = lauxh_optboolean(L, 3, 1);
    size_t pathbuf_siz = (size_t)lua_tointeger(L, lua_upvalueindex(1));
    // buf is NULL when _PC_PATH_MAX is indeterminate; realpath(3) will malloc
    char *buf          = lua_touserdata(L, lua_upvalueindex(2));
    void **gcvar       = NULL;

    lua_settop(L, 1);
    // perform normalization only (does not use the path buffer)
    if (!resolve) {
        return lua_realpath_normalize(L, (char *)path, len);
    }

    // when a pre-allocated buffer is available, check the input length
    if (buf && len > pathbuf_siz) {
        lua_pushnil(L);
        errno = ENAMETOOLONG;
        lua_errno_new(L, errno, "realpath");
        return 2;
    }

    // perform normalization before resolving the path
    if (normalize) {
        int rv = lua_realpath_normalize(L, (char *)path, len);
        if (rv != 1) {
            return rv;
        }
        // 'path' now points to the normalized Lua string on the stack
        path = lua_tostring(L, -1);
    }

    // for the NULL buf case, push a GC guard onto the stack before calling
    // realpath(3) so that the malloc'd result is freed even if lua_pushstring
    // throws an OOM error: the guard stays alive on the stack during the
    // pushstring call, and its __gc finalizer calls free() if unwound by
    // longjmp
    if (!buf) {
        gcvar = new_gcvar(L);
    }

    // perform path resolution
    // if buf is NULL, realpath(3) allocates the output buffer with malloc(3)
    path = realpath(path, buf);
    if (path) {
        if (buf) {
            // pre-allocated buffer: resolved == buf, no free needed
            lua_settop(L, 0);
            lua_pushstring(L, path);
            return 1;
        }

        // arm the GC guard while it is still live on the stack so that
        // lua_pushstring is covered: if it throws OOM, __gc frees resolved
        update_gcvar(gcvar, (void *)path);
        lua_pushstring(L, path);
        // pushstring succeeded: free immediately and disarm the guard
        update_gcvar(gcvar, NULL);
        return 1;
    }

    lua_settop(L, 0);
    lua_pushnil(L);
    lua_errno_new(L, errno, "realpath");
    return 2;
}

LUALIB_API int luaopen_realpath(lua_State *L)
{
    long pathmax = pathconf(".", _PC_PATH_MAX);

    lua_errno_loadlib(L);

    // register the GC guard metatable used for the NULL buf case
    init_gcmt(L);

    if (pathmax != -1) {
        size_t pathbuf_siz = (size_t)pathmax;
        // upvalue 1: path buffer size
        lua_pushinteger(L, (lua_Integer)pathbuf_siz);
        // upvalue 2: path buffer (held by closure until state closes)
        lua_newuserdata(L, pathbuf_siz);
    } else {
        // _PC_PATH_MAX is indeterminate: realpath(3) will allocate dynamically
        lua_pushinteger(L, 0); // upvalue 1: no size limit to check
        lua_pushlightuserdata(
            L, NULL); // upvalue 2: NULL signals dynamic allocation
    }
    lua_pushcclosure(L, realpath_lua, 2);

    return 1;
}
