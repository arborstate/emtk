#ifndef _SLCAN_H_
#define _SLCAN_H_

struct _slcan_state {
	uint8_t is_open;
	uint8_t speed;
	int (*speed_hook)(struct _slcan_state *s, uint8_t speed);
	int (*open_hook)(struct _slcan_state *s);
	int (*close_hook)(struct _slcan_state *s);
	int (*version_hook)(struct _slcan_state *s);
	int (*serial_hook)(struct _slcan_state *s);	
	int (*status_hook)(struct _slcan_state *s);
	int (*xmit_hook)(struct _slcan_state *s, uint8_t ext, uint32_t id, uint8_t *buf, uint8_t len);
	int (*rtr_hook)(struct _slcan_state *s, uint8_t ext, uint32_t id, uint8_t len);	
};

typedef struct _slcan_state slcan_state_t;

#endif /* _SLCAN_H_ */
