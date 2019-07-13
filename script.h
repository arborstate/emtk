#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <stdint.h>
#include <stddef.h>

#define SCRIPT_MAX_WORD_LEN 32
#define SCRIPT_STACK_DEPTH 32

#define SCRIPT_TRUE ((script_cell_t)-1)
#define SCRIPT_FALSE ((script_cell_t)0)

#define SCRIPT_FLAG_IMMEDIATE (1 << 0)

#ifndef SCRIPT_CELL_TYPE
typedef int script_cell_t;
#else
typedef SCRIPT_CELL_TYPE script_cell_t;
#endif

struct _script_state;
struct _script_word_info;

typedef void (*script_word_t)(struct _script_state *);

struct _script_word_info {
	const char *name;
	script_word_t code;
	script_cell_t param;
	uint8_t flags;
	script_cell_t nt;
	script_cell_t xt;
};

typedef struct _script_word_info script_word_info_t;

struct _script_state {
	script_cell_t stack[SCRIPT_STACK_DEPTH];
	size_t stackpos;

	script_cell_t *rstack[SCRIPT_STACK_DEPTH];
	size_t rstackpos;

	script_cell_t *ip;
	// Current XT
	script_cell_t w;

	const char *tib;
	size_t tibpos;
	size_t tiblen;

	script_cell_t base;

	uint8_t *heap;
	uint8_t *here;
	uint8_t *latest;

	uint8_t compiling;

	script_word_t accept;
};

typedef struct _script_state script_state_t;


int script_state_init(script_state_t *state, uint8_t *heap);
void script_add_words(script_state_t *state, script_word_info_t *vocab);

void script_push(script_state_t *state, script_cell_t v);
script_cell_t script_pop(script_state_t *state);
script_word_t script_word_lookup(script_state_t *state, const char *s);
int script_word_ingest(script_state_t *state, const char *s);
int script_eval_str(script_state_t *state, const char *s);
int script_eval_buf(script_state_t *state, const char *s, size_t len);

void script_next(script_state_t *state);

#define SCRIPT_CODE_WORD(x) void script_word_ ## x (script_state_t *state)

#define SCRIPT_DICT_WORD_ALIAS(x, alias) { #alias, script_word_ ## x, 0, 0, 0}
#define SCRIPT_DICT_WORD(x) SCRIPT_DICT_WORD_ALIAS(x, x)
#define SCRIPT_DICT_END { "", NULL}

#define SCRIPT_CELL_ALIGN(v) (((size_t)(v) % sizeof(script_cell_t)) ? ((v) + (sizeof(script_cell_t) - ((size_t)(v) % sizeof(script_cell_t)))) : (v))

SCRIPT_CODE_WORD(dup);
SCRIPT_CODE_WORD(drop);
SCRIPT_CODE_WORD(swap);
SCRIPT_CODE_WORD(add);
SCRIPT_CODE_WORD(is_zero);
SCRIPT_CODE_WORD(mult);
SCRIPT_CODE_WORD(pop_and_display);
SCRIPT_CODE_WORD(quit);
SCRIPT_CODE_WORD(enter);
SCRIPT_CODE_WORD(exit);
SCRIPT_CODE_WORD(lit);

#endif /* __SCRIPT_H__ */
