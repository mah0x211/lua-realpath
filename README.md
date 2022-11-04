# lua-realpath

[![test](https://github.com/mah0x211/lua-realpath/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-realpath/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/lua-realpath/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/lua-realpath)

canonicalize the pathname.


## Installation

```
luarocks install realpath
```

## Usage

```lua
local realpath = require('realpath')

-- normalize a pathname before resolving them
local pathname = assert(realpath('/foo/../bar/./../tmp', true))
print(pathname) -- /tmp

-- normalize a pathname
local normalize = require('realpath.normalize')
pathname = assert(realpath('/foo/../bar/./../unknown_pathname'))
print(pathname) -- /unknown_pathname
```


## Error Handling

the following functions return the `error` object created by https://github.com/mah0x211/lua-errno module.


## pathname, err = realpath( pathname [, normalize [, resolve]] )

canonicalize the extra `/` character and references to the `/./` and `/..` in pathnames and resolve them to abosolute pathname.

**Parameters**

- `pathname:string`: pathname string.
- `normalize:boolean`: if set to `true`, normalization will be performed before resolving the path string. (default: `false`)
- `resolve:boolean`: if set to `false`, only normalization will be performed. (default: `true`)

**Returns**

- `pathname:string`: canonicalized pathname string.
- `err:error`: error object on failure.


## pathname, err = realpath.normalize( pathname )

equivalent to `realpath( pathname, true, false )`.


### Path Normalization

The `pathname` will be normalized according to the following rules.

- If `pathname` starts with a `/`, the resolved path string will start with `/`.
- If a separator `/` is consecutive, it will be one.
- If a path segment is a `.`, it will be ignored.
- If a path segment is a `..`, it will be ignored and the previous segment will be deleted.
- The trailing-slash will be deleted.
