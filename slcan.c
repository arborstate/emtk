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

int slcan_handle_cmd(slcan_state_t *s, uint8_t *buf, size_t len) {	
	// Len includes carriage return.
	if (len < 2) {
		return 0;
	}

#define _MIN_ARG(x) if (len < x) return -1
#define _CHECK_OPEN(x) if (s->is_open != x) return -1
#define _VAL(var, offset, len) if (_be_hex_to_uint32(buf + offset, len, (uint32_t *)&var) < 0) return -1
#define _HOOK(x, ...) if (s->x ## _hook) ret = s->x ## _hook(__VA_ARGS__)

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
			return -1;
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
		uint8_t _dlc_pos;
		uint8_t _msg_len;
		
		if (cmd == 't') {
			// Standard
			_MIN_ARG(6);
			_dlc_pos = 4;
			_msg_len = buf[_dlc_pos];
			_MIN_ARG(6 + _msg_len);
			
			
		} else if (cmd == 'T') {
			// Extended
			_MIN_ARG(11);
			_dlc_pos = 9;
			_msg_len = buf[_dlc_pos];
			_MIN_ARG(6 + _msg_len);
		}

		_CHECK_OPEN(1);
		
		uint32_t can_id;
		uint8_t body[8];

		_VAL(can_id, 1, _dlc_pos);
		
		for (int i = 0; i < _msg_len; i += 2) {
			_VAL(body[i], _dlc_pos + i, 2);
		}

		_HOOK(xmit, s, cmd == 'T', can_id, buf, _msg_len);
		break;
	}

	case 'r':
	{
		// Transmit Standard RTR CAN Frame
		_MIN_ARG(6);
		_CHECK_OPEN(1);

		uint8_t can_id;
		uint8_t msg_len;
		
		_VAL(can_id, 1, 3);
		_VAL(msg_len, 4, 1);

		_HOOK(rtr, s, 0, can_id, msg_len);
		break;
	}
	
	case 'R':
	{
		// Send an RTR.
		_MIN_ARG(11);
		_CHECK_OPEN(1);

		uint8_t can_id;		
		uint8_t msg_len;

		_VAL(can_id, 1, 8);
		_VAL(msg_len, 9, 1);

		int ret = 0;

		_HOOK(rtr, s, 1, can_id, msg_len);		
		break;
	}

	case 'F':
		// Read Status Flags.
		_MIN_ARG(2);
		_CHECK_OPEN(1);
		
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
		break;

	case 'N':
		// Get Serial Number
		_MIN_ARG(2);
		break;
	default:
		// Unknown command
		ret = -1;
		break;
	}

	#undef _CHECK_OPEN	
	#undef _MIN_ARG


	return ret;
}

