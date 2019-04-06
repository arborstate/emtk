#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include "emdb.h"
#include "log.h"
#include "slcan.h"


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
_xmit_hook(slcan_state_t *s, uint8_t ext, uint32_t id, uint8_t *buf, size_t len)
{
	struct _chan *chan = (struct _chan *)s->data;

	LOG_INFO("chan %s request to xmit (%d ext) (0x%X id) (%d bytes): %s", chan->name, ext, id, len, _make_printable(buf, len));
	return 0;
}

static int
_resp_hook(slcan_state_t *s, const char *msg)
{
	struct _chan *chan = (struct _chan *)s->data;
	size_t msglen = strlen(msg);
	int ret;

	ret = write(chan->fd, msg, msglen);

	if (ret != msglen) {
		LOG_ERROR("chan %s failed to write %d bytes: %s", chan->name, msglen, strerror(errno));

		return -1;
	}

	return msglen;
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
#if 0
		// XXX - Do this for testing only.
		if (chan->inbuf[i] == '\n') {
			chan->inbuf[i] = '\r';
		}
#endif

		if (chan->inbuf[i] == '\r') {
			found = i;
			break;
		}
	}

	if (found > -1) {
		found += 1;

		LOG_INFO("channel %s found command (%d bytes): %s", chan->name, found, _make_printable(chan->inbuf, found));

		slcan_handle_cmd(&(chan->slcan), chan->inbuf, found);

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
		chan.slcan.resp_hook = _resp_hook;
		chan.slcan.xmit_hook = _xmit_hook;

		// Start with it open, since Linux will assume we are.
		chan.slcan.is_open = 1;
	}

	mainloop(&chan, 1);

	close(chan.fd);

	return 0;
}
