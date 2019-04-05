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

struct _chan {
	const char *name;
	int fd;

	char inbuf[32];
	size_t inbuf_pos;
	size_t inbuf_size;

	char outbuf[32];
	size_t outbuf_pos;
	size_t outbuf_size;

	slcan_state_t slcan;
};

static int
_version_hook(slcan_state_t *s, uint8_t *resp, size_t len)
{
	return snprintf(resp, len, "VBEEF");
}

static int
_xmit_hook(slcan_state_t *s, uint8_t ext, uint32_t id, uint8_t *buf, size_t len)
{
	struct _chan *chan = (struct _chan *)s->data;

	LOG_INFO("chan %s request to xmit (%d ext) (0x%X id) (%d bytes): %s", chan->name, ext, id, len, _make_printable(buf, len));
	return 0;
}

int
chan_ingest(struct _chan *chan)
{
	if (chan->inbuf_pos < 1) {
		LOG_WARN("got called with an empty buffer.  why?");
		return 0;
	}

	LOG_INFO("channel %s ingesting (%d bytes): %s", chan->name, chan->inbuf_pos, _make_printable(chan->inbuf, chan->inbuf_pos));

	int found = -1;

	// See if we have a CR.
	for (int i = 0; i < chan->inbuf_pos; i++) {
		// XXX - Do this for testing only.
		if (chan->inbuf[i] == '\n') {
			chan->inbuf[i] = '\r';
		}

		if (chan->inbuf[i] == '\r') {
			found = i;
			break;
		}
	}

	if (found > -1) {
		found += 1;

		LOG_INFO("channel %s found command (%d bytes): %s", chan->name, found, _make_printable(chan->inbuf, found));

		int resplen = slcan_handle_cmd(&(chan->slcan), chan->inbuf, found);

		if (resplen < 0) {
			LOG_ERROR("channel %s command error: %d", chan->name, resplen);
			if ((chan->outbuf_pos + 1) >= chan->outbuf_size) {
				LOG_ERROR("channel %s output buffer full!  dropping negative response.");
			} else {
				chan->outbuf[chan->outbuf_pos] = '\a';
				chan->outbuf_pos += 1;
			}
		} else {
			LOG_INFO("channel %s success response: %s", chan->name, _make_printable(chan->slcan._resp, resplen));
			if ((chan->outbuf_pos + resplen + 1) >= chan->outbuf_size) {
				LOG_ERROR("channel %s outbut buffer full!  dropping success response.");
			} else {
				memcpy(chan->outbuf + chan->outbuf_pos, chan->slcan._resp, resplen);
				chan->outbuf_pos += resplen;

				chan->outbuf[chan->outbuf_pos] = '\r';
				chan->outbuf_pos += 1;
			}
		}


		memmove(chan->inbuf + found, chan->inbuf, chan->inbuf_pos - found);

		chan->inbuf_pos = 0;
	}

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
				if ((chan[i].inbuf_size - chan[i].inbuf_pos) < 0) {
					LOG_WARN("channel %s has no free buffer for reading", chan[i].name);
					continue;
				}

				ret = read(fds[i].fd, chan[i].inbuf + chan[i].inbuf_pos, chan[i].inbuf_size - chan[i].inbuf_pos);
				if (ret == -1 ) {
					LOG_ERROR("failed to read channel %s: %s", chan[i].name, strerror(errno));
					return -1;
				}

				chan[i].inbuf_pos += ret;
				ret = chan_ingest(&chan[i]);

				if (ret < 0) {
					return -1;
				}

				// XXX - FIX ME - This is a naive way to flush the output buffer.
				if (chan[i].outbuf_pos != 0) {
					LOG_DEBUG("chan %s output buffer (%d bytes): %s", chan[i].name, chan[i].outbuf_pos, _make_printable(chan[i].outbuf, chan[i].outbuf_pos));
					ret = write(fds[i].fd, chan[i].outbuf, chan[i].outbuf_pos);

					if (ret < 0) {
						LOG_ERROR("channel %s failed to write: %s", chan[i].name, strerror(errno));
						return -1;
					}

					if (ret != chan[i].outbuf_pos) {
						LOG_ERROR("chan %s couldn't write all bytes!", chan[i].name, strerror(errno));
						return -1;
					}

					chan[i].outbuf_pos = 0;
				}
			}
		}
	}
}

int
usage(const char *progname)
{
	printf("%s serialPort\n", progname);

	return -1;
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
		chan.outbuf_size = sizeof(chan.outbuf);
		chan.outbuf_pos = 0;

		chan.inbuf_size = sizeof(chan.inbuf);
		chan.inbuf_pos = 0;

		slcan_init(&chan.slcan);
		chan.slcan.data = (void *)&chan;
		chan.slcan.version_hook = _version_hook;
	}

	mainloop(&chan, 1);

	close(chan.fd);

	return 0;
}
