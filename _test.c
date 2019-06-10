#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "util.h"
#include "slcan.h"
#include "log.h"
#include "script.h"


#define QUOTE(str) #str

#define _P(str) _make_printable(str, strlen(str))

int _be_hex_to_uint32(uint8_t *buf, size_t len, uint32_t *ret);

int
_test_hex(void)
{
#define _CHECK_POS(x, y) \
	do {								\
		uint32_t v;						\
		if (_be_hex_to_uint32(x, strlen(x), &v) < 0) { LOG_ERROR("conversion failed"); return -1; } \
		if (v != y) { _log("FAIL", "\"" x "\" != " QUOTE(y) " it's 0x%x", v); return -1; } \
		_log("PASS", "\"" x "\" == " QUOTE(y));		\
	} while (0)

#define _CHECK_NEG(x, y)						\
	do {								\
		uint32_t v;						\
		if (_be_hex_to_uint32(x, strlen(x), &v) >= 0) { \
			_log("FAIL", "conversion succeeded when it shouldn't"); \
 			return -1;					\
		}							\
		_log("PASS", "\"" x "\" didn't convert");		\
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
	LOG_DEBUG("open hook.");
	return 0;
}

int _close_hook(slcan_state_t *s) {
	LOG_DEBUG("close hook.");
	return 0;
}

int _xmit_hook(slcan_state_t *s, uint8_t ext, uint32_t id, uint8_t *buf, size_t len) {
	LOG_DEBUG("xmit: %d ext 0x%0X id %d len", ext, id, len);
	for (int i = 0; i < len; i++) {
		LOG_DEBUG("\t%d: 0x%0X", i, buf[i]);
	}

	return 0;
}

int _rtr_hook(slcan_state_t *s, uint8_t ext, uint32_t id, size_t len) {
	LOG_DEBUG("rtr: %d ext 0x%0X id %d len", ext, id, len);

	return 0;
}

int _resp_hook(slcan_state_t *s, const char *resp) {
	LOG_DEBUG("sending response: '%s'", _P(resp));

	return strlen(resp);
}

int
_test_slcan(void)
{
#define _CMD_POS(cmd) do { if ((ret = slcan_handle_cmd(&s, cmd, strlen(cmd))) < 0) { _log("FAIL", "incorrect return code: %d at %d", ret, __LINE__); return -1 ; }  _log("PASS", "positive case succeeded"); } while (0)
#define _CMD_NEG(cmd) do { if ((ret = slcan_handle_cmd(&s, cmd, strlen(cmd))) >= 0) { _log("FAIL", "incurrent return code : %d at %d", ret, __LINE__); return -1; }  _log("PASS", "negative case failed"); } while (0)

	{
		int ret;
		slcan_state_t s;
		slcan_init(&s);

		(&s)->open_hook = _open_hook;
		(&s)->close_hook = _close_hook;
		(&s)->xmit_hook = _xmit_hook;
		(&s)->rtr_hook = _rtr_hook;
		(&s)->resp_hook = _resp_hook;

		LOG_INFO("testing hooks...");
		_CMD_POS("O\r");
		_CMD_POS("T0000010021133\r");
		_CMD_NEG("T0000010021\r");
		_CMD_POS("t4563112233\r");
		_CMD_POS("T12ABCDEF2AA55\r");
		_CMD_POS("r1230\r");
		_CMD_POS("C\r");
		_CMD_NEG("T0000010021133\r");
	}
#undef _CMD_NEG
#undef _CMD_POS

	return 0;
}

int
_test_script(void)
{
#define _W(x) if (script_word_ingest(&state, #x) != 0) return -1
	script_state_t state;

	script_state_init(&state);

	_W(0x1);

	_W(dup);
	_W(+);

	_W(0xDEADBEEF);

	_W(swap);

	_W(.);
	_W(dup);
	_W(1);
	_W(+);

	_W(swap);

	_W(.);

	_W(drop);

	script_eval(&state, "3 4 + .");

	LOG_DEBUG("script stackpos is %d", state.stackpos);

	return 0;
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

	if (_test_script() != 0) {
		return -1;
	}

	return 0;
}
