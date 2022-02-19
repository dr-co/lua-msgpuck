#!./t/luaselect
-- vim: set ft=lua :

require 't.fixpath'


local mp = require 'dr.msgpuck'
local mplua = require 'MessagePack'
local b = require 'benchmark'

local test_data = {
    id      = '2ff817e6-a21c-11ec-a991-271886a1864e',
    name    = 'Vasya',
    midname = 'Ivanovich',
    surname = 'Pupkin',

    card    = {
        age     = 45,
        nick    = 'VP',
        avatar  = 'https://host.domain.ru/path/to/image.png',
        email   = 'vasya@pupkin.ru',
    },

    car     = {
        brand   = 'AutoVAZ',
        model   = 'Vesta',
        number  = 'E404KX404',
    },
}


local ITERATIONS = 5e5
local reporter = b.bm(20, b.FORMAT, true)

reporter:report(
	function()
		for i = 1, ITERATIONS do
                    mp.decode(mp.encode(test_data))
                end
	end,
	'dr.msgpuck'
)

collectgarbage('collect')

reporter:report(
	function()
		for i = 1, ITERATIONS do
                    mplua.unpack(mplua.pack(test_data))
                end
	end,
	'MessagePack'
)

