#include <string.h>
#include <stdint.h>

#include "log.h"
#include "isotp.h"

#define _HOOK(x, ...) if (s->x ## _hook) ret = s->x ## _hook(s, __VA_ARGS__)

static void
_isotp_buf_append(isotp_state_t *s, uint8_t *src, size_t to_copy) {
		memcpy(s->buf + s->bufpos, src, to_copy);
		s->bufpos += to_copy;
		s->sn = ((s->sn + 1) % 0xF);
}

static int
_isotp_reset(isotp_state_t *s)
{
		s->dl = 0;
		s->bufpos = 0;
		s->sn = 0;
		s->state = STATE_IDLE;
}

static int
_isotp_send_fc(isotp_state_t *s)
{
	// XXX - For now, ask for all frames, with no further FC, and no delays between blocks.
	int ret = 0;
	uint8_t fc[] = {(FRAME_FLOW_CONTROL << 4 | 0x00), 0, 0};
	size_t to_send = sizeof(fc) / sizeof(fc[0]);

	LOG_DEBUG("isotp %s sending flow control...", s->name);
	_HOOK(xmit, 1, s->dest_can_id, fc, to_send);
	if (ret < 0) {
		LOG_ERROR("isotp %s failed to send flow control frame", s->name);
		_isotp_reset(s);
	}

	return ret;
}

int
isotp_init(isotp_state_t *s) {
	memset(s, 0, sizeof(isotp_state_t));
	s->name = "<unknown>";
	s->pci_pos = 0;
	
	_isotp_reset(s);

	return 0;
}

int
isotp_ingest_frame(isotp_state_t *s, uint8_t *buf, uint8_t len)
{
#define _HOOK(x, ...) if (s->x ## _hook) ret = s->x ## _hook(s, __VA_ARGS__)
#define _ERROR(x) do { ret = x; goto out; } while (0)
#define _MIN_LEN(x) if (len < (s->pci_pos + x)) _ERROR(-1)
	
	int ret = 0;
	
	// XXX - We need to handle extended and mixed addressing.
	size_t data_start = s->pci_pos + 1;
	if (len < data_start) {
		LOG_WARN("isotp %s received short CAN frame", s->name);
		ret = -1;
		goto out;
	}

	uint8_t pci = buf[s->pci_pos];
	uint8_t frame_type = (pci >> 4) & 0xF;

	LOG_DEBUG("isotp %s received frame type %d", s->name, frame_type);

	switch (frame_type) {
	case FRAME_SINGLE:
		if (s->state != STATE_IDLE) {
			// Signal an unexpected receive, and clear state.
			_isotp_reset(s);			
		}

		s->state = STATE_RECV;
		
		s->dl = pci & 0xF;

		if (s->dl > (8 - data_start)) {
			LOG_ERROR("isotp %s received a single frame with too large (%d) of a length", s->name, s->dl);
			ret = -1;
			goto out;
		}
			
		_HOOK(recv, buf + data_start, s->dl);
		_isotp_reset(s);
		
		goto out;
		break;
	case FRAME_FIRST:
		if (s->state != STATE_IDLE) {
			// Signal an unexpected receive, and clear state.
			LOG_ERROR("isotp %s received a first frame while not idle.  resetting reception.", s->name);
			_isotp_reset(s);
		}

		s->state = STATE_RECV;
		
		_MIN_LEN(2);
		data_start++;

		s->dl = ((buf[s->pci_pos] & 0xF) << 8) | buf[s->pci_pos + 1];

		size_t to_copy = 8 - data_start;
		if ((s->dl - s->bufpos) < to_copy) {
			LOG_ERROR("isotp %s received a short first frame (%d) which is invalid", s->name, s->dl);
			_isotp_reset(s);
			goto out;
		}

		_isotp_buf_append(s, buf + data_start, to_copy);
		
		_isotp_send_fc(s);
		break;
	case FRAME_CONSECUTIVE:
	{
		if (s->state != STATE_RECV) {
			// Signal that we got a consecutive frame, when we shouldn't have.
			LOG_WARN("isotp %s received a consecutive frame with no first frame", s->name);
			goto out;
		}

		uint8_t new_sn = pci & 0xF;
		
		if (new_sn != s->sn) {
			LOG_ERROR("isotp %s missed a packet expected %d got %d", s->sn, new_sn);
			_isotp_reset(s);
			goto out;
		}

		size_t to_copy = s->dl - s->bufpos;
		
		if (to_copy > (8 - data_start)) {
			to_copy = 8 - data_start;
		}

		_isotp_buf_append(s, buf + data_start, to_copy);		
		
		break;
	}
	default:
		// Ignore anything else, but mention it:
		LOG_WARN("isotp %s ignoring frame %d", s->name, frame_type);
		break;
	}

	// We have a complete buffer, send it up the stack.
	if (s->dl != 0 && s->bufpos == s->dl) {
		_HOOK(recv, s->buf, s->dl);
		_isotp_reset(s);
	}
	
out:
	return ret;

#undef _MIN_LEN
#undef _ERROR
}

#undef _HOOK

