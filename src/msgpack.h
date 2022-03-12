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

#ifndef __DR_MSGPACK_H_INCLUDED__
#define __DR_MSGPACK_H_INCLUDED__


#include "msgpuck.h"


extern inline char *
mp_encode_number(char *data, double num)
{
	int64_t inum = num;

	if (inum == num) {
		if (inum < 0)
			return mp_encode_int(data, inum);
		return mp_encode_uint(data, inum);
	}
	return mp_encode_double(data, num);
}

extern inline size_t
mp_sizeof_number(double num)
{
	int64_t inum = num;

	if (inum == num) {
		if (inum < 0)
			return mp_sizeof_int(inum);
		return mp_sizeof_uint(inum);
	}
	return mp_sizeof_double(num);
}

#endif /* __DR_MSGPACK_H_INCLUDED__ */
