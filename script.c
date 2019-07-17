#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "log.h"
#include "script.h"

SCRIPT_CODE_WORD(restart)
{
	LOG_DEBUG("restarting script environment");
	state->stackpos = 0;
	state->rstackpos = 0;

	state->tib = NULL;
	state->tibpos = 0;
	state->tiblen = 0;

	state->base = 10;

	state->compiling = 0;
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

SCRIPT_CODE_WORD(dovar)
{
	script_push(state, (script_cell_t)((script_cell_t *)state->w + 1));
}

SCRIPT_CODE_WORD(docol)
{
	state->rstack[state->rstackpos] = state->ip;
	state->rstackpos += 1;

	state->ip = (script_cell_t *)state->w + 1;
}

SCRIPT_CODE_WORD(append_docol)
{
	script_cell_t *here = (script_cell_t *)state->here;

	*here = (script_cell_t)script_word_docol;
	state->here += sizeof(script_cell_t);
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
	state->ip += 1;
}

SCRIPT_CODE_WORD(rel)
{
	_STACK(0) = (script_cell_t)((uint8_t *)state->ip + *state->ip);
	_STACKINC(1);
	state->ip += 1;
}

SCRIPT_CODE_WORD(base)
{
	_STACK(0) = (script_cell_t)&(state->base);
	_STACKINC(1);
}

SCRIPT_CODE_WORD(dp)
{
	script_push(state, (script_cell_t)&(state->here));
}

SCRIPT_CODE_WORD(tib)
{
	_STACK(0) = (script_cell_t)state->tib;
	_STACKINC(1);
}


SCRIPT_CODE_WORD(tiblen)
{
	_STACK(0) = (script_cell_t)&(state->tiblen);
	_STACKINC(1);
}

SCRIPT_CODE_WORD(tibpos)
{
	_STACK(0) = (script_cell_t)&(state->tibpos);
	_STACKINC(1);
}

SCRIPT_CODE_WORD(compiling)
{
	_STACK(0) = (script_cell_t)&(state->compiling);
	_STACKINC(1);
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

SCRIPT_CODE_WORD(quote_comma)
{
	uint8_t count = _STACK(1);
	const char *s = (const char *)_STACK(2);

	_STACKINC(-2);

	*state->here = count;
	state->here += 1;

	for (size_t i = 0; i < count; i++) {
		*state->here = *s;
		s += 1;
		state->here += 1;
	}
}


SCRIPT_CODE_WORD(over)
{
	_STACK(0) = _STACK(2);
	_STACKINC(1);
}

script_word_info_t
_word_find(script_state_t *state, const char *s, size_t slen)
{
	script_word_info_t info = {0};

	uint8_t *link = state->link;
	while (link != NULL) {
		uint8_t *name = link + sizeof(script_cell_t) + 2;
		uint8_t count = *(name - 1);
		uint8_t flags = *(name - 2);

		info.flags = flags;

		if (slen == count) {
			size_t i = 0;

			// See if we match.
			while (i < count && (s[i] == name[i])) {
				i += 1;
			}

			// We found a word.
			if (i == count) {
				// This is the XT.
				info.xt = (script_cell_t)SCRIPT_CELL_ALIGN(name + count);
				info.nt = (script_cell_t)link;

				return info;
			}
		}

		link = *(uint8_t **)link;
	}

        // No match found.
	return info;
}

SCRIPT_CODE_WORD(find_name)
{
	script_cell_t count = _STACK(1);
	const char *s = (const char *)_STACK(2);

	script_word_info_t info = _word_find(state, s, count);

	_STACKINC(-1);

	if (info.nt != 0) {
		_STACK(1) = info.nt;
	} else {
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

SCRIPT_CODE_WORD(comment_line)
{
	while ((state->tibpos < state->tiblen) && state->tib[state->tibpos] != '\n') {
		state->tibpos += 1;
	}
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
	if (state->type) {
		state->type(state);
	} else {
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
}

SCRIPT_CODE_WORD(next)
{
	state->w = *state->ip;
	state->ip += 1;

	script_word_t code = *(script_word_t *)state->w;
	code(state);
}

SCRIPT_CODE_WORD(zero_branch)
{
	script_cell_t dest = *state->ip;
	_STACKINC(-1);

	if (_STACK(0) == 0) {
		state->ip = (script_cell_t *)((uint8_t *)state->ip + dest);
	} else {
		state->ip += 1;
	}
}

SCRIPT_CODE_WORD(branch)
{
	script_cell_t dest = *state->ip;
	state->ip = (script_cell_t *)((uint8_t *)state->ip + dest);
}

SCRIPT_CODE_WORD(link)
{
	_STACK(0) = (script_cell_t)&state->link;
	_STACKINC(1);
}

SCRIPT_CODE_WORD(latest)
{
	_STACK(0) = (script_cell_t)&state->latest;
	_STACKINC(1);
}

SCRIPT_CODE_WORD(interp_mode)
{
	state->compiling = 0;
}

SCRIPT_CODE_WORD(compile_mode)
{
	state->compiling = 1;
}

SCRIPT_CODE_WORD(align)
{
	state->here = SCRIPT_CELL_ALIGN(state->here);
}

SCRIPT_CODE_WORD(aligned)
{
	_STACK(1) = SCRIPT_CELL_ALIGN(_STACK(1));
}

SCRIPT_CODE_WORD(words)
{
	script_cell_t *link = (script_cell_t *)state->link;
	char buf[64];
	uint8_t *p;

	while (link != NULL) {
		p = (uint8_t *)link;
		p += sizeof(script_cell_t);

		uint8_t flags = *p;
		p += 1;

		uint8_t count = *p;

		p += 1;

		for (size_t i = 0; i < count; i++) {
			buf[i] = *p;

			p+= 1;
		}

		buf[count] = '\0';

		p = SCRIPT_CELL_ALIGN(p);
		LOG_INFO("link %p xt %p: %s", link, p, buf);
		link = (script_cell_t *)*link;
	}



}

SCRIPT_CODE_WORD(rpop)
{
	state->rstackpos -= 1;
	_STACK(0) = (script_cell_t)state->rstack[state->rstackpos];
	_STACKINC(1);
}

SCRIPT_CODE_WORD(rpush)
{
	_STACKINC(-1);
	state->rstack[state->rstackpos] = (script_cell_t *)_STACK(0);
	state->rstackpos += 1;
}

SCRIPT_CODE_WORD(rfetch)
{

	_STACK(0) = (script_cell_t)state->rstack[state->rstackpos - 1];
	_STACKINC(1);
}

SCRIPT_CODE_WORD(and)
{
	script_cell_t v = _STACK(1) & _STACK(2);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(or)
{
	script_cell_t v = _STACK(1) | _STACK(2);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(xor)
{
	script_cell_t v = _STACK(1) ^ _STACK(2);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(invert)
{
	_STACK(1) = ~_STACK(1);
}

SCRIPT_CODE_WORD(lshift)
{
	script_cell_t v = _STACK(2) << _STACK(1);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(rshift)
{
	script_cell_t v = _STACK(2) >> _STACK(1);

	_STACKINC(-1);
	_STACK(1) = v;
}
SCRIPT_CODE_WORD(lt)
{
	script_cell_t v = _STACK(2) < _STACK(1);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(lteq)
{
	script_cell_t v = _STACK(2) <= _STACK(1);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(gt)
{
	script_cell_t v = _STACK(2) > _STACK(1);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(gteq)
{
	script_cell_t v = _STACK(2) >= _STACK(1);

	_STACKINC(-1);
	_STACK(1) = v;
}

SCRIPT_CODE_WORD(accept)
{
	if (state->accept) {
		state->accept(state);
	} else {
		// Return nothing.;
		_STACKINC(-1);
		_STACK(1) = 0;
	}
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

const script_word_info_t script_words_def[] = {
	SCRIPT_DICT_WORD(restart),
	{ "//", script_word_comment_line },
	{ "cell", script_word_docon, sizeof(script_cell_t) },
	SCRIPT_DICT_WORD(dp),
	SCRIPT_DICT_WORD(lit),
	SCRIPT_DICT_WORD(rel),
	SCRIPT_DICT_WORD_ALIAS(zero_branch, 0branch),
	SCRIPT_DICT_WORD(branch),
	SCRIPT_DICT_WORD_ALIAS(fetch, @),
	SCRIPT_DICT_WORD_ALIAS(store, !),
	SCRIPT_DICT_WORD_ALIAS(cfetch, c@),
	SCRIPT_DICT_WORD_ALIAS(cstore, c!),
	SCRIPT_DICT_WORD_ALIAS(rpop, r>),
	SCRIPT_DICT_WORD_ALIAS(rpush, >r),
	SCRIPT_DICT_WORD_ALIAS(rfetch, r@),
	SCRIPT_DICT_WORD(dup),
	SCRIPT_DICT_WORD(drop),
	SCRIPT_DICT_WORD(over),
	SCRIPT_DICT_WORD(swap),
	SCRIPT_DICT_WORD_ALIAS(and, &),
	SCRIPT_DICT_WORD_ALIAS(or, |),
	SCRIPT_DICT_WORD_ALIAS(xor, ^),
	SCRIPT_DICT_WORD_ALIAS(invert, ~),
	SCRIPT_DICT_WORD_ALIAS(add, +),
	SCRIPT_DICT_WORD_ALIAS(sub, -),
	SCRIPT_DICT_WORD_ALIAS(lshift, <<),
	SCRIPT_DICT_WORD_ALIAS(rshift, >>),
	SCRIPT_DICT_WORD_ALIAS(lt, <),
	SCRIPT_DICT_WORD_ALIAS(lteq, <=),
	SCRIPT_DICT_WORD_ALIAS(gt, >),
	SCRIPT_DICT_WORD_ALIAS(gteq, >=),
	SCRIPT_DICT_WORD_ALIAS(is_zero, 0=),
	SCRIPT_DICT_WORD_ALIAS(mult, *),
	SCRIPT_DICT_WORD_ALIAS(divmod, /mod),
	SCRIPT_DICT_WORD_ALIAS(pop_and_display, .),
	SCRIPT_DICT_WORD_ALIAS(stack_dump, .s),
	SCRIPT_DICT_WORD(base),
	SCRIPT_DICT_WORD_ALIAS(tib, tib),
	SCRIPT_DICT_WORD_ALIAS(tiblen, #tib),
	SCRIPT_DICT_WORD_ALIAS(tibpos, in>),
	SCRIPT_DICT_WORD(compiling),
	SCRIPT_DICT_WORD(dp),
	{ ",", script_word_comma },
	{ "c,", script_word_char_comma },
	{ "\",", script_word_quote_comma },
	SCRIPT_DICT_WORD_ALIAS(find_name, find-nt),
	SCRIPT_DICT_WORD(execute),
	SCRIPT_DICT_WORD(docon),
	SCRIPT_DICT_WORD(docol),
	SCRIPT_DICT_WORD(dovar),
	{ "docol,", script_word_append_docol },
	SCRIPT_DICT_WORD(exit),
	SCRIPT_DICT_WORD_ALIAS(parse_name, parse-name),
	SCRIPT_DICT_WORD(type),
	SCRIPT_DICT_WORD(link),
	SCRIPT_DICT_WORD(latest),
	{ "[", script_word_interp_mode, 0, SCRIPT_FLAG_IMMEDIATE, 0},
	{ "]", script_word_compile_mode, 0, 0, 0},
	SCRIPT_DICT_WORD(align),
	SCRIPT_DICT_WORD(aligned),
	SCRIPT_DICT_WORD(words),
	SCRIPT_DICT_WORD(accept),
	SCRIPT_DICT_END
};

void
script_add_words(script_state_t *state, const script_word_info_t *vocab)
{
	while (vocab->code != NULL) {
		// Patch in the previous word's info.
		*(script_cell_t *)state->here = (script_cell_t)state->link;

		// Make ourselves the link.
		state->link = (uint8_t *)state->here;
		state->here += sizeof(script_cell_t);

		// Append the flags.
		*(state->here) = vocab->flags;
		state->here += 1;

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
	LOG_INFO("using script heap 0x%p", heap);
	script_word_restart(state);

	state->heap = heap;
	state->here = heap;
	state->link = NULL;
	state->latest = NULL;
	state->ip = NULL;

	script_add_words(state, script_words_def);
	return 0;
}

int
script_word_ingest(script_state_t *state, const char *s)
{
	static script_cell_t _lit_xt = 0;

	script_word_info_t info = _word_find(state, s, strlen(s));
	script_cell_t xt = info.xt;

	// Did we find the word in the dictionary?
	if (xt != 0) {
		if (state->compiling && !(info.flags & SCRIPT_FLAG_IMMEDIATE)) {
			script_cell_t *here = (script_cell_t *)state->here;
			*here = xt;
			state->here += sizeof(script_cell_t);
		} else {
			state->ip = (script_cell_t *)&xt;

			do {
				script_word_next(state);
			} while (state->rstackpos != 0);
		}

		return 0;
	}

	// Or Is this a number?
	{
		errno = 0;

		char *endptr;

		script_cell_t v = strtoul(s, &endptr, state->base);

		if (*endptr == '\0' && errno == 0) {
			if (state->compiling) {
				if (_lit_xt == 0) {
					const char *lit_name = "lit";
					script_word_info_t info = _word_find(state, lit_name, strlen(lit_name));
					_lit_xt = info.xt;

					if (_lit_xt == 0) {
						LOG_ERROR("failed to find lit xt.");
						return -1;
					}
				}

				script_cell_t *here = (script_cell_t *)state->here;
				// Append the handler for the literal we want on the stack.
				*here = _lit_xt;
				here += 1;

				// Append the actual literal.
				*here = v;
				here += 1;

				state->here += sizeof(script_cell_t) * 2;
			} else {
				script_push(state, v);
			}
			return 0;
		}
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
					script_word_restart(state);
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
