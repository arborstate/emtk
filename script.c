#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "log.h"
#include "script.h"

#define SCRIPT_CODE_WORD_EXPR(name, expr)				\
	SCRIPT_CODE_WORD(name)						\
	{								\
		script_push(state, (script_cell_t)expr);		\
	}

#define SCRIPT_USER_VAR(name) SCRIPT_CODE_WORD_EXPR(name, &(state->name))

SCRIPT_USER_VAR(base);
SCRIPT_USER_VAR(tibpos);
SCRIPT_USER_VAR(tiblen);
SCRIPT_USER_VAR(compiling);
SCRIPT_USER_VAR(dp);
SCRIPT_USER_VAR(sp);
SCRIPT_USER_VAR(sp0);
SCRIPT_USER_VAR(rp);
SCRIPT_USER_VAR(rp0);
SCRIPT_USER_VAR(latest);
SCRIPT_USER_VAR(current);
SCRIPT_USER_VAR(norder);
SCRIPT_USER_VAR(context);

SCRIPT_CODE_WORD_EXPR(cell, sizeof(script_cell_t));
SCRIPT_CODE_WORD_EXPR(tib, state->tib);
SCRIPT_CODE_WORD_EXPR(pad, state->pad);

SCRIPT_CODE_WORD(restart)
{
	LOG_DEBUG("restarting script environment");

	state->sp = state->sp0;
	state->rp = state->rp0;
	state->base = 10;
	state->compiling = 0;
	state->tibpos = state->tiblen;
}

#define _STACK(pos) *(state->sp - pos)
#define _STACKINC(v) (state->sp += v)

#define _RSTACK(pos) *(state->rsp - pos)
#define _RSTACKINC(v) (state->rsp += v)

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
	char buf[(sizeof(script_cell_t) * 8)];

	size_t count = _cell2str(v, state->base, buf, sizeof(buf));

	script_push(state, (script_cell_t)buf);
	script_push(state, count);
	script_word_type(state);
}


SCRIPT_CODE_WORD(stack_dump)
{
	char buf[sizeof(script_cell_t) * 8];
	size_t count;

	size_t depth = (state->sp - state->sp0);

	script_cell_t *sp = state->sp0;

	for (size_t i = 0; i < depth ; i++) {
		count = _cell2str(*sp, state->base, buf, sizeof(buf));
		sp++;

		script_push(state, (script_cell_t)buf);
		script_push(state, count);
		script_word_type(state);
		script_push(state, (script_cell_t)" ");
		script_push(state, 1);
		script_word_type(state);

	}
}

SCRIPT_CODE_WORD(dovar)
{
	script_push(state, (script_cell_t)((script_cell_t *)state->w + 1));
}

SCRIPT_CODE_WORD(docol)
{
	*state->rp = state->ip;
	state->rp++;

	state->ip = (script_cell_t *)state->w + 1;
}

SCRIPT_CODE_WORD(exit)
{
	state->rp--;
	state->ip = *state->rp;
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

SCRIPT_CODE_WORD(comma)
{
	script_cell_t v = _STACK(1);
	_STACKINC(-1);

	script_cell_t *p = (script_cell_t *)state->dp;

	*p = v;
	state->dp += sizeof(script_cell_t);
}

SCRIPT_CODE_WORD(char_comma)
{
	script_cell_t v = _STACK(1);
	_STACKINC(-1);

	uint8_t *p = (uint8_t *)state->dp;

	*p = v & 0xFF;
	state->dp += sizeof(uint8_t);
}

SCRIPT_CODE_WORD(quote_comma)
{
	uint8_t count = _STACK(1);
	const char *s = (const char *)_STACK(2);

	_STACKINC(-2);

	*state->dp = count;
	state->dp += 1;

	for (size_t i = 0; i < count; i++) {
		*state->dp = *s;
		s += 1;
		state->dp += 1;
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

	uint8_t *link = (uint8_t *)*(state->current);
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

SCRIPT_CODE_WORD(get_current)
{
	_STACK(0) = (script_cell_t)state->current;
	_STACKINC(1);
}

SCRIPT_CODE_WORD(set_current)
{
	state->current = (script_wordlist_t *)script_pop(state);
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
	state->dp = SCRIPT_CELL_ALIGN(state->dp);
}

SCRIPT_CODE_WORD(aligned)
{
	_STACK(1) = SCRIPT_CELL_ALIGN(_STACK(1));
}

SCRIPT_CODE_WORD(words)
{
	script_cell_t *link = (script_cell_t *)*(state->current);
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
	state->rp--;
	_STACK(0) = (script_cell_t)*state->rp;
	_STACKINC(1);
}

SCRIPT_CODE_WORD(rpush)
{
	_STACKINC(-1);
	*state->rp = (script_cell_t *)_STACK(0);
	state->rp++;
}

SCRIPT_CODE_WORD(rfetch)
{

	_STACK(0) = (script_cell_t)*(state->rp - 1);
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

SCRIPT_CODE_WORD(negate)
{
	_STACK(1) = -_STACK(1);
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

void
script_add_words(script_state_t *state, const script_word_info_t *vocab)
{

	while (vocab->code != NULL) {
		// Patch in the previous word's info.
		*(script_cell_t *)state->dp = (script_cell_t)*(state->current);

		// Make ourselves the link.
		*(state->current) = (script_wordlist_t)state->dp;
		state->dp += sizeof(script_cell_t);

		// Append the flags.
		*(state->dp) = vocab->flags;
		state->dp += 1;

		// Put down the word's name.
		uint8_t count = strlen(vocab->name);
		*(state->dp) = count;
		state->dp += 1;

		for (size_t i = 0; i < count; i++) {
			*state->dp = vocab->name[i];
			state->dp += 1;
		}

		// Align to a cell boundary.
		state->dp = SCRIPT_CELL_ALIGN(state->dp);

		// Put down the code address.
		*(script_cell_t *)state->dp = (script_cell_t)vocab->code;
		state->dp += sizeof(script_cell_t);

		// XXX - For code words, this only supports 1 CELL.
		// Put down the param address.
		*(script_cell_t *)state->dp = vocab->param;
		state->dp += sizeof(script_cell_t);

		vocab++;
	}
}

int
script_state_init(script_state_t *state)
{
	LOG_INFO("word info size %d", sizeof(script_word_info_t));

	memset(state, 0, sizeof(state));
	script_word_restart(state);

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
			script_cell_t *here = (script_cell_t *)state->dp;
			*here = xt;
			state->dp += sizeof(script_cell_t);
		} else {
			state->ip = (script_cell_t *)&xt;

			do {
				script_word_next(state);
			} while (state->rp != state->rp0);
		}

		return 0;
	}

	// Or Is this a number?
	{
		errno = 0;

		char *endptr;
		script_cell_t base = state->base;

		if (strlen(s) > 2) {
			if (strncmp(s, "0x", 2) == 0) {
				s += 2;
				base = 16;
			}
		}

		script_cell_t v = strtoul(s, &endptr, base);

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

				script_cell_t *here = (script_cell_t *)state->dp;
				// Append the handler for the literal we want on the stack.
				*here = _lit_xt;
				here += 1;

				// Append the actual literal.
				*here = v;
				here += 1;

				state->dp += sizeof(script_cell_t) * 2;
			} else {
				script_push(state, v);
			}
			return 0;
		}
	}

#ifdef DEBUG
	LOG_ERROR("failed to ingest word '%s': %s", s, strerror(errno));
#else
	LOG_ERROR("failed to ingest word '%s'", s);
#endif

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

SCRIPT_CODE_WORD(quit)
{
	char buf[256];
	int ret = 0;

	while (ret == 0) {
		script_push(state, (script_cell_t)buf);
		script_push(state, sizeof(buf));
		state->accept(state);

		size_t tiblen = script_pop(state);
		ret = script_eval_buf(state, buf, tiblen);

		const char *prompt = " ok\n";

		if (state->compiling) {
			prompt = " compiled\n";
		}

		if (ret != 0) {
			script_word_restart(state);
			prompt = " abort\n";
		}

		script_push(state, (script_cell_t)prompt);
		script_push(state, strlen(prompt));
		state->type(state);
	}

	return;
}
