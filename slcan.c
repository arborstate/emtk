#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "slcan.h"

int
_be_hex_to_uint32(uint8_t *buf, size_t len, uint32_t *ret) {
	uint32_t v;

	if (len < 1) {
		return -1;
	}

	*ret = 0;

	for (int i = 0; i < len; i++) {
		if (buf[i] >= '0' && buf[i] <= '9') {
			v = buf[i] - '0';
		} else if (buf[i] >= 'A' && buf[i] <= 'F') {
			v = buf[i] - 'A' + 10;
		} else if (buf[i] >= 'a' && buf[i] <= 'f') {
			v = buf[i] - 'a' + 10;
		} else {
			return -1;
		}

		*ret |= (v & 0xF) << ((len - i - 1) * 4);
	}

	return 0;
}

int
slcan_init(slcan_state_t *s)
{
	memset(s, 0, sizeof(slcan_state_t));
	return 0;
}

int
slcan_handle_cmd(slcan_state_t *s, uint8_t *buf, size_t len)
{
	// Len includes carriage return.
	if (len < 2) {
		return 0;
	}

#define _HOOK(x, ...) if (s->x ## _hook) ret = s->x ## _hook(__VA_ARGS__)
#define _ERROR(x) do { _HOOK(resp, s, "\a"); return x; } while (0)
#define _MIN_ARG(x) if (len < x) _ERROR(-1)
#define _CHECK_OPEN(x) if (s->is_open != x) _ERROR(-2)
#define _VAL(var, offset, len) if (_be_hex_to_uint32(buf + offset, len, &var) < 0) _ERROR(-3)

	int ret = 0;

	uint8_t cmd = buf[0];
	switch (cmd) {
	case 'S':
	{
		// Set interface speed.
		_MIN_ARG(3);
		_CHECK_OPEN(0);

		uint8_t new_speed = buf[1];

		switch (new_speed) {
		case 0:
			// 10k
		case 1:
			// 20k
		case 2:
			// 50k
		case 3:
			// 100k
		case 4:
			// 125k
		case 5:
			// 250k
		case 6:
			// 500k
		case 7:
			// 800k
		case 8:
			// 1m
		default:
			_ERROR(-1);
		}

		_HOOK(speed, s, new_speed);

		if (ret == 0) {
			s->speed = new_speed;
		}

		break;
	}

	case 's':
	{
		// Set BTR register.
		_MIN_ARG(6);
		_CHECK_OPEN(0);
		break;
	}

	case 'O':
	{
		// Open the channel.
		_MIN_ARG(2);

		_HOOK(open, s);

		if (ret == 0) {
			s->is_open = 1;
		}

		break;
	}

	case 'C':
	{
		// Close the channel.
		_MIN_ARG(2);

		_HOOK(close, s);

		if (ret == 0) {
			s->is_open = 0;
		}

		break;
	}

	case 't':
	case 'T':
	{
		// Transmit CAN Frame
		uint32_t _dlc_pos;
		uint32_t _msg_len;

		if (cmd == 't') {
			// Standard
			_MIN_ARG(6);
			_dlc_pos = 4;
			_VAL(_msg_len, _dlc_pos, 1);
			_MIN_ARG(6 + _msg_len);
		} else if (cmd == 'T') {
			// Extended
			_MIN_ARG(11);
			_dlc_pos = 9;
			_VAL(_msg_len, _dlc_pos, 1);
			_MIN_ARG(11 + _msg_len);
		}

		_CHECK_OPEN(1);

		uint32_t can_id;
		uint32_t t;
		uint8_t body[8];

		_VAL(can_id, 1, _dlc_pos - 1);

		for (int i = 0; i < _msg_len; i += 2) {
			_VAL(t, _dlc_pos + i, 2);
			body[i] = t;
		}

		_HOOK(xmit, s, cmd == 'T', can_id, body, _msg_len);

		if (ret == 0) {
			if (cmd == 'T') {
				_HOOK(resp, s, "Z");
			} else {
				_HOOK(resp, s, "z");
			}
		}

		break;
	}

	case 'r':
	case 'R':
	{
		// Transmit RTR CAN Frame
		uint8_t can_id_len = 3;

		if (cmd == 'R') {
			can_id_len = 8;
		}

		_MIN_ARG(3 + can_id_len);
		_CHECK_OPEN(1);

		uint32_t can_id;
		uint32_t msg_len;

		_VAL(can_id, 1, can_id_len);
		_VAL(msg_len, 1 + can_id_len, 1);

		_HOOK(rtr, s, cmd == 'R', can_id, msg_len);

		if (ret == 0) {
			if (cmd == 'R') {
				_HOOK(resp, s, "Z");
			} else {
				_HOOK(resp, s, "z");
			}
		}

		break;
	}

	case 'F':
		// Read Status Flags.
		_MIN_ARG(2);
		_CHECK_OPEN(1);

		_HOOK(status, s);

		if (ret == 0) {
			_HOOK(resp, s, "F00");
		}

		break;

	case 'M':
		// Set Acceptance Flags
		_MIN_ARG(10);

		break;
	case 'm':
		// Set Acceptance Mask
		_MIN_ARG(10);
		break;
	case 'V':
		// Get Version Number
		_MIN_ARG(2);
		_HOOK(resp, s, "V0000");
		break;

	case 'N':
		// Get Serial Number
		_MIN_ARG(2);
		_HOOK(resp, s, "N0000");
		break;
	default:
		// Unknown command
		ret = -1;
		break;
	}

	#undef _CHECK_OPEN
	#undef _MIN_ARG


	if (ret == 0) {
		_HOOK(resp, s, "\r");
	}

	return ret;

}
