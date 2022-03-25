# NAME

dr.msgpuck - Lua msgpack implementation.

The module is luaC binding of [rtsisyk/msgpuck](https://github.com/rtsisyk/msgpuck).

# SYNOPSIS

```lua

local mp = require 'dr.msgpuck'

local str = mp.encode{foo = 'bar'}

local o = mp.decode(str)


```

# DESCRIPTION

## API

### `encode(object)`

Encode object to messagepack string.

example:

```lua
local mp = require 'dr.msgpuck'

local data, error_code = mp.encode{a = 'b'}

if error_code then
	error('Can not encode messagepack')
end

return data
```

### `decode(str)`

Decode msgpack to Lua object.

example:

```lua
local mp = require 'dr.msgpuck'

local o, error_code = mp.decode(str)

if error_code ~= nil then
	error('Can not decode messagepack')
end

return o
```

### `check(str)`

Check if buffer contains decodeable messagepack.

example:

```lua
local mp = require 'dr.msgpuck'

local res = mp.check(str)

if res == nil then
	error('Invalid buffer object)
end

if res then
	-- buffer contains messagepack and additional res bytes
else
	-- buffer doesn't contain valid messagepack
end

```

### Error codes

- `ER_ARG_REQUIRED` - Argument required
- `ER_ARG_ISNOTSTRING` - Argument is not a string.
- `ER_INVALID_MESSAGEPACK` - Invalid messagepack.
- `ER_SERIALIZE` - Object can't be serialized as messagepack (for example function).
- `ER_DEEP_RECURSION` - Deep recursion.
- `ER_ARG_MODE` - Wrong `mode` option.
- `ER_ERRNO_<code>` - Libc error (ENOMEM).


## Maps & Array

To encode table as map or array You can use metatable:

```lua
mp.encode(setmetatable({1,2,3}, {__serialize = 'map'}))  -- messagepack map
mp.encode(setmetatable({1}, {__serialize = 'seq'}))	 -- messagepack array

-- the function can return serializable object
mp.encode(setmetatable(o, {__serialize = function(x) return tonumber(x) end}))

```

## Decoding stream(s)

From time to time You need decoding an element from top of buffer and
getting tail of bufer. You can use the format of `:decode`:

```lua

local mp = require 'dr.msgpuck'

local buffer = mp.encode(1) .. "hello, world!"

print(mp.decode(buffer))	-- 1
print(mp.decode{buffer})	-- 1

local res = mp.decode{buffer, mode='tail'}
print(res[1])			-- 1
print(res[2])			-- hello, world!
print(type(res))		-- table


local res = mp.decode{buffer, mode='tailpos'}
print(res[1])			-- 1
print(res[2])			-- 1
print(string.sub(buffer, res))	-- hello, world!
print(type(res))		-- table


```

# BENCHMARKS

```txt
# luajit
dr.msgpuck     system: 6.920    user: 0.390     total: 7.310    real: 7.313
MessagePack    system: 21.690   user: 0.920     total: 22.610   real: 22.625

# lua5.1
dr.msgpuck     system: 9.320    user: 0.650     total: 9.970    real: 10.031
MessagePack    system: 49.870   user: 1.230     total: 51.100   real: 51.156

# tarantool
dr.msgpuck     system: 6.070    user: 0.430     total: 6.500    real: 6.500
MessagePack    system: 24.090   user: 1.100     total: 25.190   real: 25.234

# resty
dr.msgpuck     system: 7.710    user: 0.390     total: 8.100    real: 8.125
MessagePack    system: 25.300   user: 0.840     total: 26.140   real: 26.219
```
