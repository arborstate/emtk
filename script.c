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

	state->tib = NULL;
	state->tibpos = 0;
	state->tiblen = 0;

	state->base = 10;
}

#define _STACK(pos) state->stack[state->stackpos - pos]
#define _STACKINC(v) state->stackpos += v

SCRIPT_CODE_WORD(store)
{
	script_cell_t *p = (script_cell_t *)_STACK(1);
	script_cell_t v = _STACK(2);
	_STACKINC(-2);

	*p = v;
}

SCRIPT_CODE_WORD(fetch)
{
	script_cell_t *p = (script_cell_t *)_STACK(1);
	_STACK(1) = *p;
}


SCRIPT_CODE_WORD(cstore)
{
	uint8_t *p = (uint8_t *)_STACK(1);
	script_cell_t v = _STACK(2);
	_STACKINC(-2);

	*p = v;
}

SCRIPT_CODE_WORD(cfetch)
{
	uint8_t *p = (uint8_t *)_STACK(1);
	_STACK(1) = *p;
}

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

SCRIPT_CODE_WORD(sub)
{
	script_cell_t v = _STACK(2) - _STACK(1);

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

SCRIPT_CODE_WORD(divmod)
{
	script_cell_t divisor = _STACK(1);
	script_cell_t dividend = _STACK(2);

	_STACK(1) = dividend / divisor;
	_STACK(2) = dividend % divisor;
}


static int _cell2str(script_cell_t v, script_cell_t base, char *output, size_t outputlen)
{
	// What?
	if (outputlen < 1) {
		return 0;
	}

	// Just Do It
	if (v == 0) {
		output[0] = '0';
		return 1;
	}

	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char buf[(sizeof(script_cell_t) * 8)];
	size_t i = 0;

	while ((v != 0) && (i < sizeof(buf))) {
		buf[i] = digits[v % base];
		v = v / base;
		i++;
	}

	size_t ncopy = i;

	if (ncopy > outputlen) {
		ncopy = outputlen - 3;
	}

	size_t j;

	for (j = 0; j < ncopy; j++) {
		output[j] = buf[i - j - 1];
	}

	if (ncopy != i) {
		output[j] = '.';
		j++;
		output[j] = '.';
		j++;
		output[j] = '.';
		j++;
	}

	return j;
}

SCRIPT_CODE_WORD(pop_and_display)
{
	script_cell_t v = script_pop(state);
	char buf[(sizeof(script_cell_t) * 8) + 1];

	size_t count = _cell2str(v, state->base, buf, sizeof(buf) - 1);
	buf[count] = '\0';

	LOG_INFO("%s", buf);
}


SCRIPT_CODE_WORD(stack_dump)
{
	char buf[(sizeof(script_cell_t) * 8) + 1];
	size_t count;

	for (size_t i = 0; i < state->stackpos; i++) {
		count = _cell2str(state->stack[i], state->base, buf, sizeof(buf) - 1);
		buf[count] = '\0';
		LOG_INFO("%d: %s", i, buf);
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
	state->ip = state->rstack[state->rstackpos];
}

SCRIPT_CODE_WORD(lit)
{
	_STACK(0) = *state->ip;
	_STACKINC(1);
	state->ip++;
}

SCRIPT_CODE_WORD(base)
{
	_STACK(0) = (script_cell_t)&(state->base);
	_STACKINC(1);
}

SCRIPT_CODE_WORD(here)
{
	script_push(state, (script_cell_t)state->here);
}

SCRIPT_CODE_WORD(comma)
{
	script_cell_t v = _STACK(1);
	_STACKINC(-1);

	script_cell_t *p = (script_cell_t *)state->here;

	*p = v;
	state->here += sizeof(script_cell_t);
}

SCRIPT_CODE_WORD(char_comma)
{
	script_cell_t v = _STACK(1);
	_STACKINC(-1);

	uint8_t *p = (uint8_t *)state->here;

	*p = v & 0xFF;
	state->here += sizeof(uint8_t);
}

SCRIPT_CODE_WORD(over)
{
	_STACK(0) = _STACK(2);
	_STACKINC(1);
}

static script_cell_t
_word_find(script_state_t *state, const char *s, size_t slen)
{
	uint8_t *latest = state->latest;
	while (latest != NULL) {
		uint8_t *name = latest + sizeof(script_cell_t) + 1;
		uint8_t count = *(name - 1);

		if (slen == count) {
			size_t i = 0;

			// See if we match.
			while (i < count && (s[i] == name[i])) {
				i += 1;
			}

			// We found a word.
			if (i == count) {
				// This is the XT.
				return (script_cell_t)SCRIPT_CELL_ALIGN(name + count);
			}
		}

		latest = *(uint8_t **)latest;
	}

        // No match found.
	return 0;
}

SCRIPT_CODE_WORD(findxt)
{
	script_cell_t count = _STACK(1);
	const char *s = (const char *)_STACK(2);

	script_cell_t xt = _word_find(state, s, count);

	if (xt != 0) {
		_STACK(2) = xt;
		_STACK(1) = SCRIPT_TRUE;
	} else {
		_STACKINC(-1);
		_STACK(1) = SCRIPT_FALSE;
	}
}

SCRIPT_CODE_WORD(execute)
{
	state->w = _STACK(1);
	_STACKINC(-1);

	script_word_t code = *(script_word_t *)state->w;
	code(state);
}

SCRIPT_CODE_WORD(docon)
{
	_STACK(0) = *(((script_cell_t *)state->w) + 1);
	_STACKINC(1);
}

SCRIPT_CODE_WORD(parse_name)
{
	const char *start = NULL;

	// Skip Leading Spaces
	while ((state->tibpos < state->tiblen) && isspace(state->tib[state->tibpos])) {
		state->tibpos += 1;
	}

	// Mark the starting point.
	start = state->tib + state->tibpos;

	// Keep going until we get a space.
	while ((state->tibpos < state->tiblen) && !isspace(state->tib[state->tibpos])) {
		state->tibpos += 1;
	}

	// Push the string onto the stack.
	script_push(state, (script_cell_t)start);
	script_push(state, (script_cell_t)((state->tib + state->tibpos) - start));
}

SCRIPT_CODE_WORD(type)
{
	size_t count = _STACK(1);
	const char *s = (const char *)_STACK(2);
	_STACKINC(-2);

	char buf[count + 1];

	size_t i;
	for (i = 0; i < count; i++) {
		buf[i] = *s;
		s++;
	}

	buf[i] = '\0';
	LOG_INFO("%s", buf);
}

SCRIPT_CODE_WORD(next)
{
	state->w = *state->ip;
	state->ip += 1;

	script_word_t code = *(script_word_t *)state->w;
	code(state);
}

SCRIPT_CODE_WORD(link)
{
	_STACK(0) = (script_cell_t)&state->latest;
	_STACKINC(1);
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
	SCRIPT_DICT_WORD_ALIAS(fetch, @),
	SCRIPT_DICT_WORD_ALIAS(store, !),
	SCRIPT_DICT_WORD_ALIAS(cfetch, c@),
	SCRIPT_DICT_WORD_ALIAS(cstore, c!),
	SCRIPT_DICT_WORD(dup),
	SCRIPT_DICT_WORD(drop),
	SCRIPT_DICT_WORD(over),
	SCRIPT_DICT_WORD(swap),
	SCRIPT_DICT_WORD_ALIAS(add, +),
	SCRIPT_DICT_WORD_ALIAS(sub, -),
	SCRIPT_DICT_WORD_ALIAS(is_zero, 0=),
	SCRIPT_DICT_WORD_ALIAS(mult, *),
	SCRIPT_DICT_WORD_ALIAS(divmod, /mod),
	SCRIPT_DICT_WORD_ALIAS(pop_and_display, .),
	SCRIPT_DICT_WORD_ALIAS(stack_dump, .s),
	SCRIPT_DICT_WORD(base),
	SCRIPT_DICT_WORD(here),
	{ ",", script_word_comma },
	{ "c,", script_word_char_comma },
	SCRIPT_DICT_WORD(findxt),
	SCRIPT_DICT_WORD(execute),
	{ "deadbeef", script_word_docon, 0xDEADBEEF },
	SCRIPT_DICT_WORD(docon),
	SCRIPT_DICT_WORD_ALIAS(parse_name, parse-name),
	SCRIPT_DICT_WORD(type),
	SCRIPT_DICT_WORD(link),
	SCRIPT_DICT_END
};

void
script_add_words(script_state_t *state, script_word_info_t *vocab)
{
	while (vocab->code != NULL) {
		// Patch in the previous word's info.
		*(script_cell_t *)state->here = (script_cell_t)state->latest;

		// Make ourselves the latest.
		state->latest = (uint8_t *)state->here;
		state->here += sizeof(script_cell_t);

		// Put down the word's name.
		uint8_t count = strlen(vocab->name);
		*(state->here) = count;
		state->here += 1;

		for (size_t i = 0; i < count; i++) {
			*state->here = vocab->name[i];
			state->here += 1;
		}

		// Align to a cell boundary.
		state->here = SCRIPT_CELL_ALIGN(state->here);

		// Put down the code address.
		*(script_cell_t *)state->here = (script_cell_t)vocab->code;
		state->here += sizeof(script_cell_t);

		// XXX - For code words, this only supports 1 CELL.
		// Put down the param address.
		*(script_cell_t *)state->here = vocab->param;
		state->here += sizeof(script_cell_t);

		vocab++;
	}
}

int
script_state_init(script_state_t *state, uint8_t *heap)
{
	LOG_INFO("word info size %d", sizeof(script_word_info_t));
	script_restart(state);

	state->heap = heap;
	state->here = heap;
	state->latest = NULL;
	state->ip = NULL;

	script_add_words(state, script_words_def);
	return 0;
}

int
script_word_ingest(script_state_t *state, const char *s)
{
	script_cell_t xt = _word_find(state, s, strlen(s));

	if (xt != 0) {
		state->ip = (script_cell_t *)&xt;

		do {
			script_word_next(state);

		} while (state->rstackpos != 0);

		return 0;
	}

	errno = 0;

	char *endptr;

	script_cell_t v = strtoul(s, &endptr, state->base);

	if (*endptr == '\0' && errno == 0) {
		script_push(state, v);
		return 0;
	}

	LOG_ERROR("failed to ingest word '%s': %s", s, strerror(errno));

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
	state->tib = s;
	state->tiblen = len;
	state->tibpos = 0;

	if (state->tiblen < 1) {
		return 0;
	}

	char word[64];
	size_t wordpos = 0;

	while (state->tibpos < state->tiblen) {
		if (isspace(state->tib[state->tibpos])) {
			state->tibpos += 1;

			if (wordpos != 0) {
				word[wordpos] = '\0';

				if (script_word_ingest(state, word) != 0) {
					script_restart(state);
					return -1;
				}

				wordpos = 0;
			}
		} else {
			word[wordpos] = state->tib[state->tibpos];
			wordpos++;

			state->tibpos += 1;
		}
	}

	return 0;
}
