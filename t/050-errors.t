#!./t/luaselect
-- vim: set ft=lua :

require 't.fixpath'

local tap = require 'dr.tap'
local mp = require 'dr.msgpuck'


local tests = {
    {
        desc    = 'decode: No arguments',
        input   = {},
        error   = 'ER_ARG_REQUIRED',
        foo     = mp.decode,
    },
    
    {
        desc    = 'decode: Not string',
        input   = {123},
        error   = 'ER_ARG_ISNOTSTRING',
        foo     = mp.decode,
    },
    
    {
        desc    = 'decode: Empty buffer',
        input   = {''},
        error   = 'ER_INVALID_MESSAGEPACK',
        foo     = mp.decode,
    },

    {
        desc    = 'decode: part of msgpack',
        input   = {
            string.sub(
                mp.encode({ hello = 'world' }),
                1,
                5
            )
        },
        error = 'ER_INVALID_MESSAGEPACK',
        foo     = mp.decode,
    },
    
    {
        desc    = 'encode: unserializable type',
        input   = {function() end},
        error   = 'ER_SERIALIZE',
        foo     = mp.encode,
    },
}

tap.plan(#tests)
for _, t in pairs(tests) do
    local r, e = t.foo(unpack(t.input))
    tap.is(e, t.error, t.desc)
end
