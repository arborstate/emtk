#ifndef __ISOTP_H__
#define __ISOTP_H__

enum _isotp_state_type {
	STATE_IDLE,
	STATE_RECV,
	STATE_XMIT
};

enum _isotp_frame_type {
	FRAME_SINGLE = 0,
	FRAME_FIRST = 1,
	FRAME_CONSECUTIVE = 2,
	FRAME_FLOW_CONTROL = 3,
};

typedef struct _isotp_state {
	const char *name;
	size_t pci_pos;	
	uint32_t src_can_id;
	uint32_t dest_can_id;
	
	enum _isotp_state_type state;

	uint8_t sn;
	uint8_t buf[4095];
	size_t bufpos;
	size_t dl;
	
	int (*recv_hook)(struct _isotp_state *t, uint8_t *buf, size_t len);
	int (*xmit_hook)(struct _isotp_state *t, uint8_t ext, uint32_t can_id, uint8_t *buf, size_t len);
	void *data;			 
} isotp_state_t;

int isotp_init(isotp_state_t *s);
int isotp_ingest_frame(isotp_state_t *s, uint8_t *buf, uint8_t len);

#endif /* __ISOTP_H__ */
