#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <stdint.h>
#include <stddef.h>

#define SCRIPT_STACK_DEPTH 32
struct _script_state {
	uint32_t stack[SCRIPT_STACK_DEPTH];
	size_t stackpos;
};

typedef struct _script_state script_state_t;
typedef void (*script_word_t)(struct _script_state *);

int script_state_init(script_state_t *state);
void script_push(script_state_t *state, uint32_t v);
uint32_t script_pop(script_state_t *state);
script_word_t script_word_lookup(script_state_t *state, const char *s);



#define SCRIPT_DEF_WORD(x) void script_word_ ## x (script_state_t *state)

#endif /* __SCRIPT_H__ */
