#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "slcan.h"


#define QUOTE(str) #str

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
}

int
main(void)
{
	return _test_hex();
}
