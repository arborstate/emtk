#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include "script.h"
#include "log.h"

extern script_cell_t script_dict_end;
script_state_t *_script_state;

void _accept_stdin(script_state_t *state)
{
	script_cell_t len = script_pop(state);
	uint8_t *buf = (uint8_t *)script_pop(state);

	len = read(0, buf, len);

	if (len < 0) {
		// XXX - This is the wrong thing to do.  We should abort.
		LOG_ERROR("error reading stdin: %s", strerror(errno));
		len = 0;
	}

	script_push(state, len);
}

void _type_stdout(script_state_t *state)
{
	script_cell_t len = script_pop(state);
	uint8_t *buf = (uint8_t *)script_pop(state);

	len = write(1, buf, len);

	if (len < 0) {
		// XXX - This is the wrong thing to do.  We should abort.
		LOG_ERROR("error reading stdin: %s", strerror(errno));
		len = 0;
	}
}

int
main(void)
{
	uint8_t heap[16384];

	uint8_t *dp = heap;

	script_wordlist_t *core = (script_wordlist_t *)dp;
	*core = *(script_wordlist_t *)&script_dict_end;
	dp += sizeof(script_wordlist_t);

	script_state_t *state;
	state = (script_state_t *)dp;
	LOG_INFO("state is at %p", state);
	dp += sizeof(script_state_t);

	script_state_init(state);
	state->dp = dp;
	state->context[0] = core;
	state->norder = 1;
	state->current = core;
	state->accept = _accept_stdin;
	state->type = _type_stdout;

	char tib[256];
	size_t readlen;

	while (1) {
		readlen = snprintf(tib, sizeof(tib), "quit\n");
		script_eval_buf(state, tib, readlen);
	}

	return 0;
}
