#!./t/luaselect
-- vim: set ft=lua :

require 't.fixpath'

local tap = require 'dr.tap'

tap:test(
    function(tap)
        tap:plan(6, 'check')
        local mp = require 'dr.msgpuck'

        local buf = mp.encode({hello = 'world'})
        tap:gt(#buf, #'hello', 'encoded')
        
        tap:is(mp.check(''), false,  'empty buf')

        tap:ok(mp.check(buf), 'normal buf')
        tap:is(mp.check(buf:sub(1, #'hello')), false,  'reduced buf')

        tap:ok(mp.check(buf .. mp.encode(nil) .. mp.encode(nil)), 'too long buf')
        tap:is(
            mp.check(buf .. mp.encode(nil) .. mp.encode(nil)),
            2,
            'too long buf retval')
    end
)
