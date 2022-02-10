# lua-realpath

[![test](https://github.com/mah0x211/lua-realpath/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-realpath/actions/workflows/test.yml)

canonicalize the pathname.


## Installation

```
luarocks install realpath
```

## Usage

```lua
local realpath = require('realpath')
local pathname = assert(realpath('/foo/../bar/./../tmp', {
    normalize = true,
}))
print(pathname) -- /tmp
```


## pathname, err, errno = realpath( pathname [, opts] )

canonicalize the extra `/` character and references to the `/./` and `/..` in pathnames and resolve them to abosolute pathname.

**Parameters**

- `pathname:string`: pathname string.
- `opts:table`
  - `resolve:boolean`:  if set to `false`, only normalization will be performed. (default: `true`)
  - `normalize:boolean`: if both the `normalize` and `resolve` options are set to `true`, normalization will be performed before resolving the path string. (default: `false`)

**Returns**

- `pathname:string`: canonicalized pathname string.
- `err:string`: error message on failure.
- `errno:integer`: error number.


### Path Normalization

The `pathname` will be normalized according to the following rules.

- If `pathname` starts with a `/`, the resolved path string will start with `/`.
- If a separator `/` is consecutive, it will be one.
- If a path segment is a `.`, it will be ignored.
- If a path segment is a `..`;
  - If the previous segment is not exist or is equal to `/` and `.`, it will be added.
  - Otherwise, it will be ignored and the previous segment will be deleted.
- The trailing-slash will be deleted.
