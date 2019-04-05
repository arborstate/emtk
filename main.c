#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>


#include "log.h"
#include "slcan.h"


void
_log(const char *level, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "%s: ", level);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

int
usage(const char *progname)
{
	printf("%s serialPort\n", progname);

	return -1;
}

struct _chan {
	const char *name;
	int fd;
	char buf[32];
	size_t bufpos;
	size_t bufsize;
};


static
const char *
_make_printable(uint8_t *buf, size_t nbuf)
{
	static char s[1024];
	const char hex[] = "0123456789abcdef";
	size_t pos = 0;
	int i;

	// I'm lazy. Make sure we have enough room for an escaped
	// version, and (potentially) showing that the output was
	// truncated.
	for (i = 0; i < nbuf && (pos < (sizeof(s) - 12)); i++) {
		LOG_DEBUG("pos is %d", pos);
		if ((buf[i] < 0x20) || (buf[i] > 0x7E) || buf[i] == '<' || buf[i] == '>') {
			s[pos++] = '<';
			s[pos++] = '0';
			s[pos++] = 'x';
			s[pos++] = hex[(buf[i] >> 4) & 0xF];
			s[pos++] = hex[buf[i] & 0xF];
			s[pos++] = '>';
		} else {
			s[pos++] = buf[i];
		}
	}

	if (i != nbuf) {
		// We truncated.
		s[pos++] = '<';
		s[pos++] = '.';
		s[pos++] = '.';
		s[pos++] = '.';
		s[pos++] = '>';
	}

	s[pos++] = '\0';

	return s;
}


int
chan_ingest(struct _chan *chan)
{
	if (chan->bufpos < 1) {
		LOG_WARN("got called with an empty buffer.  why?");
		return 0;
	}

	LOG_INFO("channel %s ingesting (%d bytes): %s", chan->name, chan->bufpos, _make_printable(chan->buf, chan->bufpos));

	chan->bufpos = 0;

	return 0;
}

int
mainloop(struct _chan *chan, size_t nchan)
{
	int done = 0;
	int ret;

	int nfds = 1;
	struct pollfd fds[nchan];


	for (int i = 0; i < nchan; i++) {
		fds[i].fd = chan[i].fd;
		fds[i].events = POLLIN;
		fds[i].revents = 0;
	}

	while (!done) {

		ret = poll(fds, nfds, 500);

		if (ret == 0) {
			continue;
		}

		if (ret == -1) {
			LOG_ERROR("poll failed: %s", strerror(errno));
		}

		for (int i = 0; i < nchan; i++) {
			if (fds[i].revents & POLLIN) {
				if ((chan[i].bufsize - chan[i].bufpos) < 0) {
					LOG_WARN("channel %s has no free buffer for reading", chan[i].name);
					continue;
				}

				ret = read(fds[i].fd, chan[i].buf + chan[i].bufpos, chan[i].bufsize - chan[i].bufpos);
				if (ret == -1 ) {
					LOG_ERROR("failed to read channel %s: %s", chan[i].name, strerror(errno));
					return -1;
				}

				chan[i].bufpos += ret;
				ret = chan_ingest(&chan[i]);

				if (ret < 0) {
					return -1;
				}
			}
		}
	}
}



int
main(int argc, const char *argv[])
{
	if (argc < 2) {
		return usage(argv[0]);
	}

	struct _chan chan;
	// Open the PTY/serial port.
	{
		const char *fname = argv[1];
		int iofd = open(fname, O_RDWR | O_NONBLOCK);

		LOG_DEBUG("opening %s...", fname);
		if (iofd < 0) {
			LOG_ERROR("failed to open I/O port '%s': %s", fname, strerror(errno));
			return -1;
		}

		chan.name = fname;
		chan.fd = iofd;
		chan.bufsize = sizeof(chan.buf);
		chan.bufpos = 0;
	}

	mainloop(&chan, 1);

	close(chan.fd);

	return 0;
}
