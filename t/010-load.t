#!./t/luaselect
-- vim: set ft=lua :

require 't.fixpath'

local tap = require 'dr.tap'

tap:plan(2)
local mp = require 'dr.msgpuck'
tap:isa(mp, 'table', 'module loaded')
tap:ok(mp.version() ~= nil, 'module version')

