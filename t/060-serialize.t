#!./t/luaselect
-- vim: set ft=lua :

require 't.fixpath'

local tap = require 'dr.tap'
local mp = require 'dr.msgpuck'

local tests = {
    {
	p = setmetatable({1, 2}, {__serialize = 'map'}),
	a = {1, 2}
    },
    {
	p = setmetatable({a = 123}, {__serialize = 'seq'}),
	a = {123}
    },

    {
        nomt = true,
        p = setmetatable({123}, {__serialize = function(x) return x[1] + 1 end}),
        a = 124,
    }
}

tap.plan(2 * #tests)
for _, t in pairs(tests) do
    local res = mp.decode(mp.encode(t.p))
    tap.is(res, t.a, 'result ' .. tap.dump(t.a))
    if t.nomt then
        tap.passed('skip metatable test')
    else
        tap.is(getmetatable(res), getmetatable(t.p), 'metatable')
    end
end
