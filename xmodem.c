
#include <stdlib.h>

#include "stm32f334x8.h"
#include "system.h"

#include "xmodem.h"


char
xmodem_cancel(struct xmodem_state *s)
{
	s->state = XMODEM_STATE_CANCEL;
	s->seq = 1;
	s->idx = 0;

	return 'C';
}

char
xmodem_ingest(struct xmodem_state *s, char c)
{
	char ret = 0;

	switch (s->idx) {
	case 0:
		switch (c) {
		case 0x1:
			// SOH
			s->state = XMODEM_STATE_SOH;
			s->crc = 0;
			s->packetpos = 0;
			break;
		case 0x4:
			// End of Transmission
			s->state = XMODEM_STATE_EOT;
			goto complete;
			break;
		case 0x17:
			// End of Transmission Block
			s->state = XMODEM_STATE_EOB;
			goto complete;
			break;
		case 0x18:
			// Cancel
			return xmodem_cancel(s);
			break;
		default:
			goto nack;
		}
		break;
	case 1:
		s->state = XMODEM_STATE_RECV;
		// Sequence
		if (c != s->seq) {
			goto nack;
		}
		break;
	case 2:
		// Sequence Recip
		if (c != 0xFF - (s->seq)) {
			goto nack;
		}

		break;
	case 131:
		// CRC
		if (c != ((s->crc >> 8) & 0xFF)) {
			goto nack;
		}

		break;
	case 132:
		if (c != (s->crc & 0xFF)) {
			goto nack;
		}

		s->seq += 1;

		s->state = XMODEM_STATE_EOP;

		// XXX - Use the valid packet.

		goto ack;
		break;
	default:
		// Payload
		s->crc ^= ((int)c) << 8;

		for (int i = 0; i < 8; i++) {
			if  (s->crc & 0x8000) {
				s->crc = (s->crc << 1) ^ 0x1021;
			} else {
				s->crc = s->crc << 1;
			}
		}

		s->packet[s->packetpos] = c;
		s->packetpos += 1;
		break;
	}

	s->idx += 1;

	return 0;
nack:
	ret = 0x15;
	goto next;
complete:
	s->seq = 1;
ack:
	ret = 0x06;
next:
	s->idx = 0;
	return ret;
}
