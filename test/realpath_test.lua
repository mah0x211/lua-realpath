local testcase = require('testcase')
local errno = require('error').errno
local realpath = require('realpath')

function testcase.realpath_resolve()
    -- test that resolve pathname
    for v, match in pairs({
        ['.'] = 'lua%-realpath/test$',
        ['..'] = 'lua%-realpath$',
        ['./realpath_test.lua'] = 'lua%-realpath/test/realpath_test%.lua$',
    }) do
        local pathname = assert(realpath(v))
        assert.match(pathname, match, false)
    end

    -- test that perform normalization before resolve pathname
    local pathname, err, eno = realpath('./foo/../bar/../realpath_test.lua/', {
        normalize = true,
    })
    assert.match(pathname, 'lua%-realpath/test/realpath_test%.lua$', false)
    assert.is_nil(err)
    assert.is_nil(eno)

    -- test that returns error
    pathname, err, eno = realpath('./foo/../bar/../realpath_test.lua/')
    assert.is_nil(pathname)
    assert.is_string(err)
    assert.equal(errno[eno], errno.ENOENT)
end

function testcase.realpath_normalize()
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
            "..",
        },
        {
            "../..",
            "../..",
        },
        {
            "../../abc",
            "../../abc",
        },
        {
            "/abc",
            "/abc",
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
            "..",
        },
        {
            "../../",
            "../..",
        },
        {
            "/abc/",
            "/abc",
        },

        -- Remove doubled slash
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

        -- Remove . elements
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

        -- Remove .. elements
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
            "..",
        },
        {
            "/abc/def/../../..",
            "/",
        },
        {
            "abc/def/../../../ghi/jkl/../../../mno",
            "../../mno",
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
            "../../def",
        },
    }) do
        local pathname = assert(realpath(v[1], {
            resolve = false,
        }))
        -- print(string.format('#%02d: %-2s | %-38s | %-12s | %s', _,
        --                     pathname == v[2] and '' or 'NG', v[1], pathname,
        --                     v[2]))
        assert.equal(pathname, v[2])
    end

    -- test that return error if pathname is invalid string
    local pathname, err, eno = realpath('foo/bar' .. string.char(0) .. '/baz', {
        resolve = false,
    })
    assert.is_nil(pathname);
    assert.is_string(err)
    assert.equal(errno[eno], errno.EILSEQ)
end
