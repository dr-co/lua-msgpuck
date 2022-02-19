#!./t/luaselect
-- vim: set ft=lua :

require 't.fixpath'

local tap = require 'dr.tap'

tap:plan(3)
local mp = require 'dr.msgpuck'
tap:isnt(mp.encode, nil, 'encode')

tap:is(mp.decode(mp.encode(nil)), nil, 'encode nil')

tap:test(
    function(tap)
        local tvalues = {
            { t = { ['true']            = true }, },
            { t = { ['false']           = false }, },
            { t = { ['zero']            = 0 }, },
            { t = { ['hello, wrd']      = 'hello, wrd' },  },
            { t = { ['n17']             = 17 }, },
            { t = { ['n127']            = 127 }, },
            { t = { ['n128']            = 128 }, },
            { t = { ['n-1']             = -1 }, },
            { t = { ['n-256']           = -256 }, },
            { t = { ['n1.7']            = 1.7 }, },
            { t = { ['n-1.7']           = -1.7 }, },
            { t = { ['s""']             = '' }, },
            {
                t  = { ['t{}']    = {} },
                mt = { ['__serialize']   = 'seq' }
            },
            {
                t  = { ['t{}'] =
                    setmetatable({}, {__serialize='map'})
                },
                mt = { ['__serialize']   = 'map' }
            },
            {
                t  = { ['t{}'] =
                    setmetatable({1, 2}, {__serialize='map'})
                },
                mt = { ['__serialize']   = 'map' }
            },
            {
                t  = { ['t{}']    = {1, 2, 3, 'abc'} },
                mt = { ['__serialize']   = 'seq' }
            },
            {
                t  = { ['t{a = "b"}']    = {a = 'b'} },
                mt = { ['__serialize']   = 'map' }
            },
            {
                t  = { ['t{a = "b"}']    = {1, 2, a = 'b', 3} },
                mt = { ['__serialize']   = 'map' }
            },

            {
                t = { ['object']  =
                    setmetatable(
                        { a = '1' },
                        {
                            ['__serialize'] = function(sf)
                                                 return sf.a
                                              end
                        }
                    ),
                },
                rv = '1',
            }
        }

        tap.plan(3 * #tvalues)

        for _, t in pairs(tvalues) do
            local k, v = next(t.t)
            local r, e = mp.decode(mp.encode(v))
            local await_value = t.rv
            if await_value == nil then
                await_value = v
            end

            if tap:is(e, nil, 'not error') then
                if t.mt ~= nil then
                    tap:is(
                        getmetatable(r),
                        t.mt,
                        'metatable: ' .. tap:dump(t.mt))
                else
                    tap.passed('no metatable test')
                end
                
                tap:is(r, await_value, string.format(
                    'decode(encode(%s))',
                    tap:dump(v))
                )
            end

        end
    end,
    'encode-decode'
)


