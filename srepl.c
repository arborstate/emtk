#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include "script.h"
#include "log.h"

script_state_t _script_state;

int
main(void)
{
	uint8_t heap[8192];
	char buf[1024];

	script_state_init(&_script_state, heap);

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
