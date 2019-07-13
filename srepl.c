#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include "script.h"
#include "log.h"

script_state_t _script_state;

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

int
main(void)
{
	uint8_t heap[16384];
	char buf[1024];

	script_state_init(&_script_state, heap);

	_script_state.accept = _accept_stdin;

	size_t len;
	while(1) {
		len = read(0, buf, sizeof(buf) - 1);
		
		if (len < 0) {
			LOG_ERROR("error reading stdin: %s", strerror(errno));
			return len;
		}

		if (len == 0) {
			return 0;
		}

		// LOG_DEBUG("got input '%s'", buf);
		buf[len] = '\0';

		script_eval_buf(&_script_state, buf, len);
		LOG_INFO("ok");
	}
	
	return 0;
}
