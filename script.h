#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <stdint.h>
#include <stddef.h>

#define SCRIPT_MAX_WORD_LEN 32
#define SCRIPT_STACK_DEPTH 32
#define SCRIPT_MAX_VOCAB 4

struct _script_state;
struct _script_word_info;

typedef void (*script_word_t)(struct _script_state *, struct _script_word_info *);

struct _script_word_info {
	const char *name;
	script_word_t code;
};

typedef struct _script_word_info script_word_info_t;

struct _script_state {
	uint32_t stack[SCRIPT_STACK_DEPTH];
	size_t stackpos;

	char word[SCRIPT_MAX_WORD_LEN + 1];
	size_t wordpos;

	script_word_info_t *vocab[SCRIPT_MAX_VOCAB];
	size_t vocabpos;
};

typedef struct _script_state script_state_t;


int script_state_init(script_state_t *state);
void script_add_vocab(script_state_t *state, script_word_info_t *);

void script_push(script_state_t *state, uint32_t v);
uint32_t script_pop(script_state_t *state);
script_word_info_t *script_word_lookup(script_state_t *state, const char *s);
int script_word_ingest(script_state_t *state, const char *s);
int script_eval_str(script_state_t *state, const char *s);
int script_eval_buf(script_state_t *state, const char *s, size_t len);

#define SCRIPT_DEF_WORD(x) void script_word_ ## x (script_state_t *state, script_word_info_t *info)

#endif /* __SCRIPT_H__ */
