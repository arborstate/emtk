#ifndef _XMODEM_H_
#define _XMODEM_H_

#include <stdint.h>

#define XMODEM_STATE_CANCEL	0
#define XMODEM_STATE_SOH	1
#define XMODEM_STATE_EOT	2
#define XMODEM_STATE_EOB	3
#define XMODEM_STATE_EOP	4
#define XMODEM_STATE_RECV	5

struct xmodem_state {
	uint8_t state;
	uint8_t idx;
	uint8_t seq;
	int crc;
	char packet[128];
	size_t packetpos;
};

char xmodem_ingest(struct xmodem_state *s, char c);
char xmodem_cancel(struct xmodem_state *s);

#endif /* _XMODEM_H_ */
