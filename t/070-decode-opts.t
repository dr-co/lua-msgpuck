#!./t/luaselect
-- vim: set ft=lua :

require 't.fixpath'

local tap = require 'dr.tap'

local mp = require 'dr.msgpuck'


tap.is(
    mp:decode{ mp.encode{1,2,3} }, 
    { 1, 2, 3 },
    'decode arguments as table'
)


tap.is(
    mp:decode{ mp.encode{1,2,3}, mode = 'tailpos' }, 
    {
        { 1, 2, 3 },
        1 + #mp:encode{1,2,3},
    },
    'mode = tailpos'
)


local buf = mp.encode{1,2,3} .. 'hello, world!'

tap.is(
    mp:decode{ buf, mode = 'tail' }, 
    {
        { 1, 2, 3 },
        "hello, world!"
    },
    'mode = tail'
)

tap.is(
    string.sub(
        buf,
        mp.decode({buf, mode='tailpos'})[2]
    ),
    "hello, world!",
    'mode = tail'
)
