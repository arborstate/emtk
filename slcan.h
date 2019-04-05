#ifndef _SLCAN_H_
#define _SLCAN_H_

struct _slcan_state {
	char _resp[8];
	uint8_t is_open;
	uint8_t speed;

	int (*speed_hook)(struct _slcan_state *s, uint8_t speed);
	int (*open_hook)(struct _slcan_state *s);
	int (*close_hook)(struct _slcan_state *s);
	int (*version_hook)(struct _slcan_state *s, uint8_t *resp, uint8_t len);
	int (*serial_hook)(struct _slcan_state *s, uint8_t *resp, uint8_t len);
	int (*status_hook)(struct _slcan_state *s, uint8_t *resp, uint8_t len);
	int (*xmit_hook)(struct _slcan_state *s, uint8_t ext, uint32_t id, uint8_t *buf, uint8_t len);
	int (*rtr_hook)(struct _slcan_state *s, uint8_t ext, uint32_t id, uint8_t len);

	int (*resp_hook)(struct _slcan_state *s, uint8_t *buf, uint8_t len);
};

typedef struct _slcan_state slcan_state_t;

int slcan_init(slcan_state_t *s);
int slcan_handle_cmd(slcan_state_t *s, uint8_t *buf, size_t len);

#endif /* _SLCAN_H_ */
