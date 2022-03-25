/*************************************************************************
 *                                                                       *
 * Copyright (C) 2011 Dmitry E. Oboukhov <unera@debian.org>              *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                       *
 *************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include "msgpack.h"
#include "buffer.h"

#define VERSION		"0.1-1"

#define ER_ARG_REQUIRED		-1
#define ER_ARG_ISNOTSTRING	-2
#define ER_INVALID_MESSAGEPACK	-3
#define ER_SERIALIZE		-4
#define ER_DEEP_RECURSION	-5
#define ER_ARG_MODE		-6


#define MAX_DEEP_RECURSION	10000

static __thread struct buffer buffer = {.pc = NULL, .pe = NULL, .size = 0};


static int
drlua_pusherror(lua_State *L, int error)
{
	switch (error) {
		case ER_ARG_REQUIRED:
			lua_pushliteral(L, "ER_ARG_REQUIRED");
			break;
		case ER_ARG_ISNOTSTRING:
			lua_pushliteral(L, "ER_ARG_ISNOTSTRING");
			break;
		case ER_INVALID_MESSAGEPACK:
			lua_pushliteral(L, "ER_INVALID_MESSAGEPACK");
			break;
		case ER_SERIALIZE:
			lua_pushliteral(L, "ER_SERIALIZE");
			break;
		case ER_DEEP_RECURSION:
			lua_pushliteral(L, "ER_DEEP_RECURSION");
			break;
		case ER_ARG_MODE:
			lua_pushliteral(L, "ER_ARG_MODE");
			break;
		default: {
			assert(error > 0);
			char buffer[48];
			// NOLINTNEXTLINE
			snprintf(buffer, sizeof(buffer), "ER_ERRNO_%d", error);
			lua_pushstring(L, buffer);
			break;
		}
	}
	return 1;
}



static int
dr_msgpuck_version(lua_State *L)
{
    lua_pushstring(L, VERSION);
    return 1;
}


static int
_encode_item(lua_State *L, unsigned recursion, unsigned max_recursion)
{
	const char *p;
	size_t len;
	bool bvalue;
	lua_Number nvalue;
	int error, i;
	int type = lua_type(L, -1);

	if (recursion > max_recursion)
		return ER_DEEP_RECURSION;

	switch(type) {
		case LUA_TNIL:
			BUFFER_GROW(buffer, mp_sizeof_nil());
			buffer.pe = mp_encode_nil(buffer.pe);
			break;

		case LUA_TBOOLEAN:
			bvalue = lua_toboolean(L, -1);
			BUFFER_GROW(buffer, mp_sizeof_bool(bvalue));
			buffer.pe = mp_encode_bool(buffer.pe, bvalue);
			break;

		case LUA_TSTRING:
			p = luaL_checklstring(L, -1, &len);
			BUFFER_GROW(buffer, mp_sizeof_str(len));
			buffer.pe = mp_encode_str(buffer.pe, p, len);
			break;

		case LUA_TNUMBER:
			nvalue = luaL_checknumber(L, -1);
			BUFFER_GROW(buffer, mp_sizeof_number(nvalue));
			buffer.pe = mp_encode_number(buffer.pe, nvalue);
			break;

		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA:
		case LUA_TTABLE:
			if (lua_getmetatable(L, -1)) {
				lua_pushliteral(L, "__serialize");
				lua_rawget(L, -2);

				switch(lua_type(L, -1)) {
					case LUA_TFUNCTION:
						lua_remove(L, -2);
						// stack: value, function
						lua_pushvalue(L, -2);
						// stack: value, cb, value
						lua_remove(L, -3);
						// stack: cb, value
						lua_call(L, 1, 1);
						// stack: new_value
						return _encode_item(L,
							recursion + 1,
							max_recursion);

					case LUA_TSTRING:
						if (type != LUA_TTABLE)
							return ER_SERIALIZE;
						p = luaL_checklstring(L,
							-1, &len);
						if (strncmp(p, "map", 3) == 0) {
							lua_pop(L, 2);
							goto LUA_TTABLE_map;
						}
						if (strncmp(p, "seq", 3) == 0) {
							lua_pop(L, 2);
							goto LUA_TTABLE_array;
						}
						break;
				}

				lua_pop(L, 2);
			}

			len = 0;
			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				// -2 - key, -1 - value
				lua_pop(L, 1);
				if (lua_type(L, -1) != LUA_TNUMBER) {
					lua_pop(L, 1);
					goto LUA_TTABLE_map;
				}
				len++;
			}

			if (lua_objlen(L, -1) != len)
				goto LUA_TTABLE_map;
			goto LUA_TTABLE_array;

		LUA_TTABLE_map:
			lua_pushnil(L);
			len = 0;
			while (lua_next(L, -2) != 0) {
				// -2 - key, -1 - value
				lua_pop(L, 1);
				len++;
			}

			BUFFER_GROW(buffer, mp_sizeof_map(len));
			buffer.pe = mp_encode_map(buffer.pe, len);


			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				// -2 - key, -1 - value

				lua_pushvalue(L, -2);	// key
				if ((error = _encode_item(L, recursion + 1, max_recursion)))
					return error;

				// value
				if ((error = _encode_item(L, recursion + 1, max_recursion)))
					return error;
			}

			break;

		LUA_TTABLE_array:
			lua_pushnil(L);
			len = 0;
			while (lua_next(L, -2) != 0) {
				// -2 - key, -1 - value
				lua_pop(L, 1);
				len++;
			}

			BUFFER_GROW(buffer, mp_sizeof_array(len));
			buffer.pe = mp_encode_array(buffer.pe, len);

			lua_pushnil(L);
			for (i = 1; lua_next(L, -2) != 0; i++) {
				// value
				if ((error = _encode_item(L, recursion + 1, max_recursion)))
					return error;
			}

			break;

		default:
			return ER_SERIALIZE;
	}

	lua_pop(L, 1); // drop value from stack
	return 0;
}

static int
dr_msgpuck_encode(lua_State *L)
{
	int error, top;

	buffer_init(&buffer);

	top = lua_gettop(L);

	if (!top) {
		lua_pushlstring(L, "", 0);
		return 1;
	}

	if ((error = _encode_item(L, 1, MAX_DEEP_RECURSION))) {
		lua_settop(L, top);
		lua_pushnil(L);
		drlua_pusherror(L, error);
		return 2;
	}

	lua_pushlstring(L, buffer.pc, buffer_len(&buffer));
	return 1;
}

static int
decode_item(lua_State *L, const char **p, size_t len)
{
	uint64_t uvalue;
	int64_t ivalue;
	double dvalue;
	const char *svalue;
	uint32_t slen;
	int i, error;
	const char *pe = *p + len;

	switch(mp_typeof(**p)) {
		case MP_UINT:
			uvalue = mp_decode_uint(p);
			lua_pushinteger(L, uvalue);
			break;

		case MP_INT:
			ivalue = mp_decode_int(p);
			lua_pushinteger(L, ivalue);
			break;

		case MP_NIL:
			mp_decode_nil(p);
			lua_pushnil(L);
			break;

		case MP_FLOAT:
			dvalue = mp_decode_float(p);
			lua_pushnumber(L, dvalue);
			break;

		case MP_DOUBLE:
			dvalue = mp_decode_double(p);
			lua_pushnumber(L, dvalue);
			break;

		case MP_STR:
			svalue = mp_decode_str(p, &slen);
			lua_pushlstring(L, svalue, slen);
			break;

		case MP_BOOL:
			if (mp_decode_bool(p))
				lua_pushboolean(L, true);
			else
				lua_pushboolean(L, false);
			break;

		case MP_MAP:
			lua_newtable(L);
			len = mp_decode_map(p);
			for (i = 0; i < len; i++) {
				if ((error = decode_item(L, p, pe - *p)))
					return error;
				if ((error = decode_item(L, p, pe - *p)))
					return error;
				lua_rawset(L, -3);
			}

			lua_newtable(L);
			lua_pushliteral(L, "__serialize");
			lua_pushliteral(L, "map");
			lua_rawset(L, -3);
			lua_setmetatable(L, -2);
			break;

		case MP_ARRAY:
			lua_newtable(L);
			len = mp_decode_array(p);
			for (i = 0; i < len; i++) {
				lua_pushinteger(L, i + 1);
				if ((error = decode_item(L, p, pe - *p)))
					return error;
				lua_rawset(L, -3);
			}

			lua_newtable(L);
			lua_pushliteral(L, "__serialize");
			lua_pushliteral(L, "seq");
			lua_rawset(L, -3);
			lua_setmetatable(L, -2);
			break;

		case MP_BIN:
		case MP_EXT:
			break;

	}
	return 0;
}

static int
dr_msgpuck_decode(lua_State *L)
{
	int error;
	size_t len;
	const char *p, *ps;
	int top = lua_gettop(L);
	enum {
		NORMAL = 0,
		TAIL_POS,
		TAIL,
	} mode = NORMAL;

	if (!top) {
		error = ER_ARG_REQUIRED;
		goto on_error;

	}

	switch(lua_type(L, -1)) {
		case LUA_TSTRING:
			p = luaL_checklstring(L, -1, &len);
			break;

		case LUA_TTABLE:
			lua_pushinteger(L, 1);
			lua_rawget(L, -2);
			if (lua_type(L, -1) != LUA_TSTRING) {
				error = ER_ARG_ISNOTSTRING;
				goto on_error;
			}


			lua_pushliteral(L, "mode");
			lua_rawget(L, -3);
			if (lua_type(L, -1) == LUA_TSTRING) {
				p = luaL_checklstring(L, -1, &len);
				if (strcmp(p, "tailpos") == 0) {
					mode = TAIL_POS;
				} else if (strcmp(p, "tail") == 0) {
					mode = TAIL;
				} else {
					error = ER_ARG_MODE;
					goto on_error;
				}
			}
			lua_pop(L, 1);

			p = luaL_checklstring(L, -1, &len);

			break;

		default:
			error = ER_ARG_ISNOTSTRING;
			goto on_error;
	}


	if (!len) {
		error = ER_INVALID_MESSAGEPACK;
		goto on_error;
	}

	ps = p;

	/* check buffer length */
	if (mp_check(&ps, ps + len) != 0) {
		error = ER_INVALID_MESSAGEPACK;
		goto on_error;
	}

	ps = p;
	error = decode_item(L, &p, len);

	if (error) {
		lua_settop(L, top);
		lua_pop(L, 1);
		goto on_error;
	}

	switch(mode) {
		case NORMAL:
			break;

		case TAIL_POS:
			lua_newtable(L);
			lua_pushinteger(L, 1);
			lua_pushvalue(L, -3);
			lua_rawset(L, -3);
			lua_remove(L, -2);

			lua_pushinteger(L, 2);
			lua_pushinteger(L, 1 + p - ps);
			lua_rawset(L, -3);
			break;

		case TAIL:
			lua_newtable(L);
			lua_pushinteger(L, 1);
			lua_pushvalue(L, -3);
			lua_rawset(L, -3);
			lua_remove(L, -2);

			lua_pushinteger(L, 2);
			lua_pushlstring(
				L,
				ps + ((int)(p - ps)), len - (p - ps)
			);
			lua_rawset(L, -3);
			break;

	}

	return 1;

	on_error:
		lua_pushnil(L);
		drlua_pusherror(L, error);
		return 2;
}


int
dr_msgpuck_check(lua_State *L)
{
	size_t len;
	const char *p, *ps;
	int rc;

	if (!lua_gettop(L)) {
		lua_pushnil(L);
		return 1;
	}
	if (lua_type(L, -1) != LUA_TSTRING) {
		lua_pushnil(L);
		return 1;
	}
	p = luaL_checklstring(L, -1, &len);

	ps = p;
	rc = mp_check(&ps, ps + len);

	if (rc == 0) {
		lua_pushinteger(L, len - (ps - p));
		return 1;
	}

	lua_pushboolean(L, false);
	return 1;
}


int
luaopen_dr_msgpuck(lua_State *L)
{
	static const struct luaL_Reg lib [] = {
		{"version", dr_msgpuck_version},
		{"encode", dr_msgpuck_encode},
		{"decode", dr_msgpuck_decode},
		{"check", dr_msgpuck_check},
		{NULL, NULL}
	};
	lua_newtable(L);
	for (int i = 0; lib[i].name; i++) {
		lua_pushstring(L, lib[i].name);
		lua_pushcclosure(L, lib[i].func, 0);
		lua_settable(L, -3);
	}
	return 1;
}

