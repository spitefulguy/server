#ifndef UTILS_H_
#define UTILS_H_

const static char STATIC_EXT[] = "jpg jpeg gif txt ico js css html";

const char *weekday(int year, int month, int day);
char* get_extension(const char *filename);

#endif /* UTILS_H_ */
