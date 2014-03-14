#include "stdio.h"
#include "stdlib.h"
#include "utils.h"
#include "string.h"
#include "ctype.h"

#include <stdbool.h>
#include <stdint.h>


int get_extension(char *ext_buf, const char *filename) {
	int len = strlen(filename);

	int i = len;
	while (*(filename + i) != '.') {
		if ((filename + i) == filename) return 0; else i--;
	} ;
	i++;

	int ext_len = len - i;

	strncpy(ext_buf, (filename + i), ext_len);

	ext_buf[ext_len] = '\0';

	int j = ext_len;
	while (--j > 0)
		*(ext_buf + j) = tolower(*(ext_buf + j));

	return ext_len;
}

static char hex_decode_char(uint8_t c) {
	return isdigit(c) ? c - '0' : c - 'a' + 10;
}

static char hex_decode(uint8_t hi, uint8_t lo) {
	return (hex_decode_char(hi) << 4) + hex_decode_char(lo);
}

static bool is_hex_char(int c) {
	return (isdigit(c) || (c <= 'f' && c >= 'a'));
}
int percent_decode(const char *in_buff, int in_len, char* out_buff, int out_len) {
	int in_pos, out_pos;
	in_pos = out_pos = 0;

	while(in_pos < in_len) {
		if(out_pos >= out_len)
			return -1;

		char c = in_buff[in_pos++];
		if(c == '%') {
			if(!(in_pos + 1 < in_len))
				return -1;
			char h_hi = tolower(in_buff[in_pos++]);
			char h_lo = tolower(in_buff[in_pos++]);
			if(!is_hex_char(h_hi) || !is_hex_char(h_lo))
				return -1;
			out_buff[out_pos++] = hex_decode(h_hi, h_lo);
		} else {
			out_buff[out_pos++] = c;
		}
	}
	return out_pos;
}
