#ifndef __RINGBUF_H__
#define __RINGBUF_H__
#include <stddef.h>

#define RINGBUF_DEF(type, size) \
	struct ringbuf_ ## type ## _size_ ## size {	\
		type buf[size];				\
		size_t wpos;				\
		size_t rpos;				\
	}

#define RINGBUF_INST(name, type, size)				\
	volatile struct ringbuf_ ## type ## _size_ ## size name

#define RINGBUF_DIST(name) \
	(((name.rpos + sizeof(name.buf)) - name.wpos) % sizeof(name.buf))

#define RINGBUF_INC(name, field) \
	name.field = (name.field + 1) % sizeof(name.buf)

#endif /* __RINGBUF_H__ */
