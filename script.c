#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "log.h"
#include "script.h"

#define _STACK(pos) state->stack[state->stackpos - pos]
#define _STACKINC(v) state->stackpos += v

SCRIPT_DEF_WORD(dup)
{
	_STACK(0) = _STACK(1);
	_STACKINC(1);
}

SCRIPT_DEF_WORD(drop)
{
	_STACKINC(-1);
}

SCRIPT_DEF_WORD(swap)
{
	uint32_t a = _STACK(1);
	uint32_t b = _STACK(2);

	_STACK(1) = b;
	_STACK(2) = a;
}

SCRIPT_DEF_WORD(add)
{
	uint32_t v = _STACK(1) + _STACK(2);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_DEF_WORD(is_zero)
{
	_STACK(1) = _STACK(1) == 0 ? -1 : 0;
}

SCRIPT_DEF_WORD(mult)
{
	_STACKINC(-1);
	_STACK(1) = _STACK(1) * _STACK(0);
}

int
script_state_init(script_state_t *state)
{
	state->stackpos = 0;
}

void
script_push(script_state_t *state, uint32_t v) {
	_STACK(0) = v;
	_STACKINC(1);
}

uint32_t
script_pop(script_state_t *state) {
	_STACKINC(-1);

	return _STACK(0);
}

script_word_t
script_word_lookup(script_state_t *state, const char *s)
{
#define _S_ALIAS(x, alias) if (strcmp(s, #alias) == 0) return script_word_ ## x
#define _S(x) _S_ALIAS(x, x)
	_S(dup);
	_S(drop);
	_S(swap);
	_S_ALIAS(add, +);
	_S_ALIAS(is_zero, 0=);
	_S_ALIAS(mult, *);
#undef _S
#undef _S_ALIAS

	// No match found.
	return NULL;
}

#undef _STACKINC
#undef _STACK

int
script_word_ingest(script_state_t *state, const char *s)
{
	script_word_t w;

	w = script_word_lookup(state, s);

	if (w != NULL) {
		w(state);

		return 0;
	}

	errno = 0;

	char *endptr;

	uint32_t v = strtol(s, &endptr, 0);

	if (errno != 0) {
		LOG_ERROR("failed to convert literal '%s': %s", s, strerror(errno));
		return -1;
	}

	script_push(state, v);

	return 0;
}

int
script_eval(script_state_t *state, const char *s)
{
	char word[32];
	size_t wordpos = 0;
	int inword = 0;

	while (*s != '\0') {
		if (isspace(*s)) {
			if (inword) {
				word[wordpos] = '\0';

				if (script_word_ingest(state, word) != 0) {
					return -1;
				}

				wordpos = 0;
				inword = 0;
			}
		} else {
			word[wordpos] = *s;
			wordpos += 1;
			inword = 1;
		}

		s += 1;
	}

	// Leftovers.
	if (inword) {
		word[wordpos] = '\0';

		if (script_word_ingest(state, word) != 0) {
			return -1;
		}
	}

	return 0;
}
