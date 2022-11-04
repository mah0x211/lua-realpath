/**
 *  Copyright (C) 2022 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#ifndef lua_realpath_h
#define lua_realpath_h

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// lua
#include <lua_errno.h>

static inline int lua_realpath_normalize(lua_State *L, char *path, size_t len)
{
    int top       = 0;
    lua_State *th = lua_newthread(L);
    char *head    = path;
    char *tail    = path + len;
    char *p       = head;

CHECK_FIRST:
    switch (*p) {
    case '/':
ADD_SEGMENT:
        // add segments
        luaL_checkstack(th, 2, NULL);
        if ((uintptr_t)head < (uintptr_t)p) {
            lua_pushlstring(th, head, (uintptr_t)p - (uintptr_t)head);
            top++;
        }
        // add '/'
        lua_pushliteral(th, "/");
        top++;

        // skip multiple slashes
        while (*p == '/') {
            p++;
        }
        head = p;
        if (*p != '.') {
            break;
        }
        // fallthrough

    case '.':
        if (p[1] == '/' || p[1] == 0) {
            // foud '.' segment
            p += 1;
        } else if (p[1] == '.' && (p[2] == '/' || p[2] == 0)) {
            // found '..' segment
            p += 2;
            if (top > 1) {
                // remove previous segment with trailing-slash
                lua_pop(th, 2);
                top -= 2;
            }
        } else {
            // allow segments that started with '.' character
            break;
        }

        // skip multiple slashes
        while (*p == '/') {
            p++;
        }
        head = p;
        goto CHECK_FIRST;
    }

    // search '/' character
    while (*p) {
        if (*p == '/') {
            goto ADD_SEGMENT;
        }
        p++;
    }

    // found NULL before the end of the string
    if (p != tail) {
        lua_pushnil(L);
        errno = EILSEQ;
        lua_errno_new(L, errno, "normalize");
        return 2;
    }

    // add last-segment
    if ((uintptr_t)head < (uintptr_t)tail) {
        top++;
        luaL_checkstack(th, 1, NULL);
        lua_pushlstring(th, head, (uintptr_t)tail - (uintptr_t)head);
    }

    // return '.' segment if empty
    if (!top) {
        lua_pushliteral(L, ".");
        return 1;
    }

    // remove trailing-slash
    if (top > 1 && *(char *)lua_tostring(th, top) == '/') {
        lua_pop(th, 1);
        top--;
    }

    // return a normalized path string
    lua_replace(L, 1);
    lua_settop(L, 1);
    lua_concat(th, top);
    lua_xmove(th, L, 1);

    return 1;
}

#endif
