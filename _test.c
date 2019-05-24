#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "util.h"
#include "slcan.h"


#define QUOTE(str) #str

#define _P(str) _make_printable(str, strlen(str))

int _be_hex_to_uint32(uint8_t *buf, size_t len, uint32_t *ret);

int
_test_hex(void)
{
#define _CHECK_POS(x, y) \
	do {								\
		uint32_t v;						\
		if (_be_hex_to_uint32(x, strlen(x), &v) < 0) { printf("conversion failed\n"); return -1; } \
		if (v != y) { printf("FAIL \"" x "\" != " QUOTE(y) " it's 0x%x\n", v); return -1; } \
		printf("PASS \"" x "\" == " QUOTE(y) "\n");		\
	} while (0)

#define _CHECK_NEG(x, y)						\
	do {								\
		uint32_t v;						\
		if (_be_hex_to_uint32(x, strlen(x), &v) >= 0) { \
			printf ("FAIL conversion succeeded when it shouldn't\n"); \
 			return -1;					\
		}							\
		printf("PASS \"" x "\" didn't convert\n");		\
	} while (0)

	_CHECK_POS("A", 0xa);
	_CHECK_POS("1235", 0x1235);
	_CHECK_POS("AeF1", 0xaef1);
	_CHECK_NEG("S", 0xaef1);
	_CHECK_NEG("123G", 0xaef1);
	_CHECK_NEG("", 0xaef1);

#undef _CHECK_NEG
#undef _CHECK_POS

	return 0;
}

int _open_hook(slcan_state_t *s) {
	printf("open hook.\n");
	return 0;
}

int _close_hook(slcan_state_t *s) {
	printf("close hook.\n");
	return 0;
}

int _xmit_hook(slcan_state_t *s, uint8_t ext, uint32_t id, uint8_t *buf, size_t len) {
	printf("xmit: %d ext 0x%0X id %d len\n", ext, id, len);
	for (int i = 0; i < len; i++) {
		printf("\t%d: 0x%0X\n", i, buf[i]);
	}

	return 0;
}

int _rtr_hook(slcan_state_t *s, uint8_t ext, uint32_t id, size_t len) {
	printf("rtr: %d ext 0x%0X id %d len\n", ext, id, len);

	return 0;
}

int _resp_hook(slcan_state_t *s, const char *resp) {
	printf("sending response: '%s'\n", _P(resp));

	return strlen(resp);
}

int
_test_slcan(void)
{
#define _CMD_POS(cmd) do { if ((ret = slcan_handle_cmd(&s, cmd, strlen(cmd))) < 0) { printf("FAIL: incorrect return code: %d at %d\n", ret, __LINE__); return -1 ; }  printf("PASS: positive case succeeded\n"); } while (0)
#define _CMD_NEG(cmd) do { if ((ret = slcan_handle_cmd(&s, cmd, strlen(cmd))) >= 0) { printf("FAIL: incurrent return code : %d at %d\n", ret, __LINE__); return -1; }  printf("PASS: negative case failed\n"); } while (0)

	{
		int ret;
		slcan_state_t s;
		slcan_init(&s);

		(&s)->open_hook = _open_hook;
		(&s)->close_hook = _close_hook;
		(&s)->xmit_hook = _xmit_hook;
		(&s)->rtr_hook = _rtr_hook;
		(&s)->resp_hook = _resp_hook;

		printf("testing hooks...\n");
		_CMD_POS("O\r");
		_CMD_POS("T0000010021133\r");
		_CMD_NEG("T0000010021\r");
		_CMD_POS("t4563112233\r");
		_CMD_POS("T12ABCDEF2AA55\r");
		_CMD_POS("r1230\r");
		_CMD_POS("C\r");
		_CMD_NEG("T0000010021133\r");
	}
#undef _CMD_POS
}

int
main(void)
{

	if (_test_hex() != 0) {
		return -1;
	}

	if (_test_slcan() != 0) {
		return -1;
	}

	return 0;
}
