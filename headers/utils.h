#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <stdbool.h>



typedef struct {
	uint8_t set[16];
} reserved_set;

int percent_decode(const char *in_buff, int in_len, char* out_buff, int out_len);

extern const reserved_set all_reserved_set;
extern const reserved_set conservative_reserved_set;
extern const reserved_set liberal_reserved_set;

void mark_unreserved(reserved_set* set, char c);
void mark_reserved(reserved_set* set, char c);
bool is_reserved(const reserved_set* set, char c);

int get_extension(char *ext_buf, const char *filename);

#endif /* UTILS_H_ */
