#include "stdio.h"
#include "stdlib.h"
#include "utils.h"
#include "string.h"
#include "ctype.h"

const char *weekday(int year, int month, int day) {
  static const char *weekdayname[] = {"Monday", "Tuesday",
        "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
  size_t JND =                                                     \
          day                                                      \
        + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) \
        + (365 * (year + 4800 - ((14 - month) / 12)))              \
        + ((year + 4800 - ((14 - month) / 12)) / 4)                \
        - ((year + 4800 - ((14 - month) / 12)) / 100)              \
        + ((year + 4800 - ((14 - month) / 12)) / 400)              \
        - 32045;
  return weekdayname[JND % 7];
}

char* get_extension(const char *filename) {
	int len = strlen(filename);
	int i = len;

	while (*(filename + i) != '.') {
		if ((filename + i) == filename) return NULL; else i--;
	} ;
	i++;

	int ext_len = len - i;
	char *ext = malloc(sizeof(char)*(ext_len + 1));

	strncpy(ext, (filename + i), ext_len);

	ext[ext_len] = '\0';

	while (--ext_len > 0)
		*(ext + ext_len) = tolower(*(ext + ext_len));

	return (strstr(STATIC_EXT, ext)) ? ext : NULL;
}
