#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <stdint.h>
#include <stddef.h>

#define SCRIPT_MAX_WORD_LEN 32
#define SCRIPT_STACK_DEPTH 32
#define SCRIPT_MAX_VOCAB 4

// typedef uint32_t script_cell_t;
typedef uint64_t script_cell_t;

struct _script_state;
struct _script_word_info;

typedef void (*script_word_t)(struct _script_state *, struct _script_word_info *);

struct _script_word_info {
	const char *name;
	script_word_t code;
};

typedef struct _script_word_info script_word_info_t;

struct _script_state {
	script_cell_t stack[SCRIPT_STACK_DEPTH];
	size_t stackpos;

	script_cell_t *rstack[SCRIPT_STACK_DEPTH];
	size_t rstackpos;

	script_cell_t *ip;
	script_cell_t *w;

	char word[SCRIPT_MAX_WORD_LEN + 1];
	size_t wordpos;

	script_word_info_t *vocab[SCRIPT_MAX_VOCAB];
	size_t vocabpos;
};

typedef struct _script_state script_state_t;


int script_state_init(script_state_t *state);
void script_add_vocab(script_state_t *state, script_word_info_t *);

void script_push(script_state_t *state, script_cell_t v);
script_cell_t script_pop(script_state_t *state);
script_word_info_t *script_word_lookup(script_state_t *state, const char *s);
int script_word_ingest(script_state_t *state, const char *s);
int script_eval_str(script_state_t *state, const char *s);
int script_eval_buf(script_state_t *state, const char *s, size_t len);

void script_next(script_state_t *state);

#define SCRIPT_CODE_WORD(x) void script_word_ ## x (script_state_t *state, script_word_info_t *info)

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
