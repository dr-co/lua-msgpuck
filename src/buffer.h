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

#ifndef __DR_BUFFER_H_INCLUDED__
#define __DR_BUFFER_H_INCLUDED__


#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

struct buffer {
	char *pc;
	char *pe;
	size_t size;
};


extern inline size_t
buffer_len(const struct buffer *buf)
{
	return buf->pe - buf->pc;
}

extern inline void
buffer_init(struct buffer *buf)
{
	buf->pe = buf->pc;
}


extern inline struct buffer *
buffer_alloc(void)
{
	struct buffer *res = (struct buffer *) malloc(sizeof(struct buffer));

	if (res) {
		res->pc = NULL;
		res->pe = NULL;
		res->size = 0;
	}
	return res;
}

extern inline int
buffer_grow(struct buffer *buf, size_t new_size)
{
	if (new_size <= buf->size)
		return 0;

	if (new_size < 64)
		new_size += 32;
	else
		new_size += new_size / 5;
	char *p = (char *)realloc(buf->pc, new_size);

	if (!p)
		return errno;

	buf->pe = p + (buf->pe - buf->pc);
	buf->pc = p;
	buf->size = new_size;
	return 0;
}


#define BUFFER_GROW(buf, addsize) { \
	int __err; \
	if ((__err = buffer_grow(&(buf), (addsize) + buffer_len(&(buf))))) \
		return __err; \
	}

#endif /* __DR_BUFFER_H_INCLUDED__ */
