#include <stdint.h>
#include <stddef.h>


const char *
_make_printable(const uint8_t *buf, size_t nbuf)
{
	static char s[1024];
	const char hex[] = "0123456789abcdef";
	size_t pos = 0;
	int i;

	// I'm lazy. Make sure we have enough room for an escaped
	// version, and (potentially) showing that the output was
	// truncated.
	for (i = 0; i < nbuf && (pos < (sizeof(s) - 12)); i++) {
		if ((buf[i] < 0x20) || (buf[i] > 0x7E) || buf[i] == '<' || buf[i] == '>') {
			s[pos++] = '<';
			s[pos++] = '0';
			s[pos++] = 'x';
			s[pos++] = hex[(buf[i] >> 4) & 0xF];
			s[pos++] = hex[buf[i] & 0xF];
			s[pos++] = '>';
		} else {
			s[pos++] = buf[i];
		}
	}

	if (i != nbuf) {
		// We truncated.
		s[pos++] = '<';
		s[pos++] = '.';
		s[pos++] = '.';
		s[pos++] = '.';
		s[pos++] = '>';
	}

	s[pos++] = '\0';

	return s;
}
