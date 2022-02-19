#!./t/luaselect
-- vim: set ft=lua :

require 't.fixpath'

local tap = require 'dr.tap'

tap:plan(8)
local drmp = require 'dr.msgpuck'
tap:isnt(drmp.encode, nil, 'encode')

tap.isa(drmp.encode(nil), 'string')
tap:is(drmp.encode(nil), string.char(0xc0), 'encode nil')
tap:is(drmp.encode(false), string.char(0xc2), 'false')
tap:is(drmp.encode(true), string.char(0xc3), 'true')
tap:is(drmp.encode(0), string.char(0x00), 0)
tap:is(drmp.encode(24), string.char(0x18), 24)
tap:is(drmp.encode(-11), string.char(0xF5), -11)

