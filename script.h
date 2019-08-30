#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <stdint.h>
#include <stddef.h>

#define SCRIPT_MAX_WORD_LEN 32
#define SCRIPT_STACK_DEPTH 16

#define SCRIPT_TRUE ((script_cell_t)-1)
#define SCRIPT_FALSE ((script_cell_t)0)

#define SCRIPT_FLAG_IMMEDIATE (1 << 0)

#ifndef SCRIPT_CELL_TYPE
typedef int script_cell_t;
#else
typedef SCRIPT_CELL_TYPE script_cell_t;
#endif

typedef script_cell_t * script_wordlist_t;

struct _script_state;

typedef void (*script_word_t)(struct _script_state *);

struct _script_state {
	script_cell_t *ip;
	// Current XT
	script_cell_t w;

	script_cell_t sp0[SCRIPT_STACK_DEPTH];
	script_cell_t *sp;
	script_cell_t *rp0[SCRIPT_STACK_DEPTH];
	script_cell_t **rp;

	const char *tib;
	size_t tibpos;
	size_t tiblen;

	script_word_t accept;
	script_word_t type;

	script_cell_t base;
	script_cell_t compiling;
	uint8_t pad[sizeof(script_cell_t) * 8];

	uint8_t *dp;
	uint8_t *latest;

	script_wordlist_t *current;
	script_cell_t norder;
	script_wordlist_t *context[16];
} __attribute__((packed));

typedef struct _script_state script_state_t;


int script_state_init(script_state_t *state);

void script_push(script_state_t *state, script_cell_t v);
script_cell_t script_pop(script_state_t *state);
script_word_t script_word_lookup(script_state_t *state, const char *s);
int script_word_ingest(script_state_t *state, const char *s);
int script_eval_str(script_state_t *state, const char *s);
int script_eval_buf(script_state_t *state, const char *s, size_t len);

void script_next(script_state_t *state);

#define SCRIPT_CODE_WORD(x) void script_word_ ## x (script_state_t *state)
#define SCRIPT_CELL_ALIGN(v) (((size_t)(v) % sizeof(script_cell_t)) ? ((v) + (sizeof(script_cell_t) - ((size_t)(v) % sizeof(script_cell_t)))) : (v))

SCRIPT_CODE_WORD(quit);

#endif /* __SCRIPT_H__ */
