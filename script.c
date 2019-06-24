#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "log.h"
#include "script.h"

void
script_restart(script_state_t *state)
{
	LOG_DEBUG("restarting script environment");
	state->stackpos = 0;
	state->rstackpos = 0;
	state->wordpos = 0;
}

#define _STACK(pos) state->stack[state->stackpos - pos]
#define _STACKINC(v) state->stackpos += v

SCRIPT_CODE_WORD(dup)
{
	_STACK(0) = _STACK(1);
	_STACKINC(1);
}

SCRIPT_CODE_WORD(drop)
{
	_STACKINC(-1);
}

SCRIPT_CODE_WORD(swap)
{
	script_cell_t a = _STACK(1);
	script_cell_t b = _STACK(2);

	_STACK(1) = b;
	_STACK(2) = a;
}

SCRIPT_CODE_WORD(add)
{
	script_cell_t v = _STACK(1) + _STACK(2);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(is_zero)
{
	_STACK(1) = _STACK(1) == 0 ? -1 : 0;
}

SCRIPT_CODE_WORD(mult)
{
	_STACKINC(-1);
	_STACK(1) = _STACK(1) * _STACK(0);
}

SCRIPT_CODE_WORD(pop_and_display)
{
	script_cell_t v = script_pop(state);

	LOG_INFO("0x%X", v);
}


SCRIPT_CODE_WORD(stack_dump)
{
	for (size_t i = 0; i < state->stackpos; i++) {
		LOG_INFO("%d: 0x%X", i, state->stack[i]);
	}
}

SCRIPT_CODE_WORD(quit)
{
	script_restart(state);
}

SCRIPT_CODE_WORD(enter)
{
	state->rstack[state->rstackpos] = state->ip + 1;
	state->rstackpos += 1;

	state->ip = (script_cell_t *)*state->ip;
}

SCRIPT_CODE_WORD(exit)
{
	state->rstackpos -= 1;
}

SCRIPT_CODE_WORD(lit)
{
	_STACK(0) = *state->ip;
	_STACKINC(1);
	state->ip++;
}

void
script_push(script_state_t *state, script_cell_t v) {
	_STACK(0) = v;
	_STACKINC(1);
}

script_cell_t
script_pop(script_state_t *state) {
	_STACKINC(-1);

	return _STACK(0);
}

#undef _STACKINC
#undef _STACK

script_word_info_t script_words_def[] = {
	SCRIPT_DICT_WORD(quit),
	SCRIPT_DICT_WORD(dup),
	SCRIPT_DICT_WORD(drop),
	SCRIPT_DICT_WORD(swap),
	SCRIPT_DICT_WORD_ALIAS(add, +),
	SCRIPT_DICT_WORD_ALIAS(is_zero, 0=),
	SCRIPT_DICT_WORD_ALIAS(mult, *),
	SCRIPT_DICT_WORD_ALIAS(pop_and_display, .),
	SCRIPT_DICT_WORD_ALIAS(stack_dump, .s),
	SCRIPT_DICT_END
};

void
script_add_vocab(script_state_t *state, script_word_info_t *vocab)
{
	state->vocab[state->vocabpos] = vocab;
	state->vocabpos += 1;

	state->vocab[state->vocabpos] = NULL;
}

int
script_state_init(script_state_t *state)
{
	script_restart(state);
	state->vocabpos = 0;

	script_add_vocab(state, script_words_def);
}

script_word_info_t *
script_word_lookup(script_state_t *state, const char *s)
{
	size_t curvocab = 0;

	while (state->vocab[curvocab] != 0) {
		size_t curword = 0;

		while (state->vocab[curvocab][curword].code != NULL) {
			if (strcmp(s, state->vocab[curvocab][curword].name) == 0) {
				return &state->vocab[curvocab][curword];
			}

			curword += 1;
		}

		curvocab += 1;
	}

	// No match found.
	return NULL;
}

int
script_word_ingest(script_state_t *state, const char *s)
{
	script_word_info_t *info;

	info = script_word_lookup(state, s);

	if (info != NULL) {
		info->code(state, info);

		return 0;
	}

	errno = 0;

	char *endptr;

	script_cell_t v = strtol(s, &endptr, 0);

	if (*endptr == '\0' && errno == 0) {
		script_push(state, v);
		return 0;
	}

	LOG_ERROR("failed to ingest word '%s'", s);

	return -1;
}

int
script_eval_str(script_state_t *state, const char *s)
{
	return script_eval_buf(state, s, strlen(s));
}

int
script_eval_buf(script_state_t *state, const char *s, size_t len)
{
	const char *end = s + len;

	while (s < end) {
		if (isspace(*s)) {
			if (state->wordpos != 0) {
				state->word[state->wordpos] = '\0';

				if (script_word_ingest(state, state->word) != 0) {
					script_restart(state);
					return -1;
				}

				state->wordpos = 0;
			}
		} else {
			state->word[state->wordpos] = *s;
			state->wordpos += 1;
		}

		s += 1;
	}

	return 0;
}
