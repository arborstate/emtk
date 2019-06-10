#include <stdint.h>
#include <string.h>

#include "log.h"
#include "script.h"

SCRIPT_DEF_WORD(dup)
{
	state->stack[state->stackpos] = state->stack[state->stackpos - 1];
	state->stackpos += 1;
}

SCRIPT_DEF_WORD(drop)
{
	state->stackpos -= 1;
}

SCRIPT_DEF_WORD(swap)
{
	uint32_t a = state->stack[state->stackpos - 1];
	uint32_t b = state->stack[state->stackpos - 2];

	state->stack[state->stackpos - 1] = b;
	state->stack[state->stackpos - 2] = a;
}

SCRIPT_DEF_WORD(add)
{
	uint32_t v = state->stack[state->stackpos - 1] + state->stack[state->stackpos - 2];

	state->stackpos -= 1;

	state->stack[state->stackpos - 1] = v;
}

int
script_state_init(script_state_t *state)
{
	state->stackpos = 0;
}

void
script_push(script_state_t *state, uint32_t v) {
	state->stack[state->stackpos] = v;
	state->stackpos += 1;
}

uint32_t
script_pop(script_state_t *state) {
	state->stackpos -= 1;

	return state->stack[state->stackpos];
}

script_word_t
script_word_lookup(script_state_t *state, const char *s)
{
#define _S(x) if (strcmp(s, #x) == 0) return script_word_ ## x
	_S(dup);
	_S(drop);
	_S(swap);
	_S(add);
#undef _S

	// No match found.
	return NULL;
}
