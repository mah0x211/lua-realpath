local assert = require('assert')
local errno = require('errno')
local realpath = require('realpath')
local normalize = require('realpath.normalize')

local function test_realpath_resolve()
    -- test that resolve pathname
    for v, match in pairs({
        ['.'] = 'lua%-realpath$',
        ['./test/..'] = 'lua%-realpath$',
        ['./test/realpath_test.lua'] = 'lua%-realpath/test/realpath_test%.lua$',
    }) do
        local pathname = assert(realpath(v))
        assert.match(pathname, match, false)
    end

    -- test that perform normalization before resolve pathname
    local pathname, err = realpath('./foo/../bar/../test/realpath_test.lua/',
                                   true)
    assert.match(pathname, 'lua%-realpath/test/realpath_test%.lua$', false)
    assert.is_nil(err)

    -- test that return error if cannot normalize pathname
    pathname, err = realpath('foo/bar' .. string.char(0) .. '/baz', true)
    assert.is_nil(pathname);
    assert.equal(err.type, errno.EILSEQ)

    -- test that returns error
    pathname, err = realpath('./foo/../bar/../realpath_test.lua/')
    assert.is_nil(pathname)
    assert.equal(err.type, errno.ENOENT)
end

local function test_realpath_normalize()
    -- test that peform normalize only
    -- NOTE: this test cases copied from
    --  https://github.com/golang/go/blob/go1.17.7/src/path/filepath/path_test.go#L26
    for _, v in ipairs({
        -- Already clean
        {
            "abc",
            "abc",
        },
        {
            "abc/def",
            "abc/def",
        },
        {
            "a/b/c",
            "a/b/c",
        },
        {
            ".",
            ".",
        },
        {
            "..",
            ".",
        },
        {
            "../..",
            ".",
        },
        {
            "../../abc",
            "abc",
        },
        {
            "/abc",
            "/abc",
        },
        {
            "../foo/.dot-file",
            "foo/.dot-file",
        },
        {
            "/",
            "/",
        },

        -- Empty is current dir
        {
            "",
            ".",
        },

        -- Remove trailing slash
        {
            "abc/",
            "abc",
        },
        {
            "abc/def/",
            "abc/def",
        },
        {
            "a/b/c/",
            "a/b/c",
        },
        {
            "./",
            ".",
        },
        {
            "../",
            ".",
        },
        {
            "../../",
            ".",
        },
        {
            "/abc/",
            "/abc",
        },

        -- Remove multiple slashes
        {
            "abc//def//ghi",
            "abc/def/ghi",
        },
        {
            "//abc",
            "/abc",
        },
        {
            "///abc",
            "/abc",
        },
        {
            "//abc//",
            "/abc",
        },
        {
            "abc//",
            "abc",
        },

        -- Remove . segments
        {
            "abc/./def",
            "abc/def",
        },
        {
            "/./abc/def",
            "/abc/def",
        },
        {
            "abc/.",
            "abc",
        },

        -- Remove .. segments
        {
            "abc/..",
            ".",
        },
        {
            "/abc/..",
            "/",
        },
        {
            "abc/def/ghi/../jkl",
            "abc/def/jkl",
        },
        {
            "abc/def/../ghi/../jkl",
            "abc/jkl",
        },
        {
            "abc/def/..",
            "abc",
        },
        {
            "abc/def/../..",
            ".",
        },
        {
            "/abc/def/../..",
            "/",
        },
        {
            "abc/def/../../..",
            ".",
        },
        {
            "/abc/def/../../..",
            "/",
        },
        {
            "abc/def/../../../ghi/jkl/../../../mno",
            "mno",
        },
        {
            "/../abc",
            "/abc",
        },

        -- Combinations
        {
            "abc/./../def",
            "def",
        },
        {
            "abc//./../def",
            "def",
        },
        {
            "abc/../../././../def",
            "def",
        },
    }) do
        local pathname = assert(realpath(v[1], nil, false))
        -- print(string.format('#%02d: %-2s | %-38s | %-12s | %s', _,
        --                     pathname == v[2] and '' or 'NG', v[1], pathname,
        --                     v[2]))
        assert.equal(pathname, v[2])
        assert.equal(normalize(v[1]), pathname)
    end

    -- test that return error if pathname is invalid string
    local pathname, err = realpath('foo/bar' .. string.char(0) .. '/baz', nil,
                                   false)
    assert.is_nil(pathname);
    assert.equal(err.type, errno.EILSEQ)

    pathname, err = normalize('foo/bar' .. string.char(0) .. '/baz', nil, false)
    assert.is_nil(pathname);
    assert.equal(err.type, errno.EILSEQ)
end

test_realpath_resolve()
test_realpath_normalize()
