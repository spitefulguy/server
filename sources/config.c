/*
 * config.c
 *
 *  Created on: Mar 14, 2014
 *      Author: alex
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_PAGE "DEFAULT_PAGE"
#define STATIC_DIR "STATIC_DIR"
#define LOCAL_PORT "LOCAL_PORT"
#define THREADS "THREADS"
#define METHODS_TO_FORBID "METHODS_TO_FORBID"
#define SEND_RETRY_INTERVAL_USEC "SEND_RETRY_INTERVAL_USEC"

static char *default_page;
static char *static_dir;
static char *methods_to_forbid;
static int local_port;
static int threads;
static int send_retry_interval_usec;

#define MAX_LINE_LENGTH 512

static int get_char_entry(const char *line, char **setting) {
	while (*(++line) == '\t' || *(line) == ' ')
		if (strchr("\n\0", *line)) return -1;

	char *p;
	p = strchr(line, '\n');
	if (!p) return -1;

	size_t length = p - line;
	*setting = malloc(sizeof(char) * (length + 1));
	strncpy(*setting, line, length);
	return length;
}

static int get_int_entry(const char *line, int *setting) {

	while (*(++line) == '\t' || *(line) == ' ')
		if (strchr("\n\0", (*line))) return -1;

	char *p;
	p = strchr(line, '\n');
	if (!p) return -1;

	size_t length = p - line;

	char *buf = malloc(sizeof(char) * (length + 1));
	strncpy(buf, line, length);
	buf[length] = '\0';
	*setting = atoi(buf);
	return length;
}


int init_config(const char *path) {
	FILE *file = fopen(path, "r");
	if (!file) {
		return -1;
	}

	char *t = malloc(sizeof(char) * (MAX_LINE_LENGTH + 1));

	while ( fgets(t, MAX_LINE_LENGTH, file) ) {

		if (strstr(t, DEFAULT_PAGE))
			get_char_entry(t + strlen(DEFAULT_PAGE), &default_page);
		else if (strstr(t, METHODS_TO_FORBID))
			get_char_entry(t += strlen(METHODS_TO_FORBID), &methods_to_forbid);
		else if (strstr(t, STATIC_DIR))
			get_char_entry(t += strlen(STATIC_DIR), &static_dir);

		else if (strstr(t, LOCAL_PORT))
			get_int_entry(t += strlen(LOCAL_PORT), &local_port);
		else if (strstr(t, THREADS))
			get_int_entry(t += strlen(THREADS), &threads);
		else if (strstr(t, SEND_RETRY_INTERVAL_USEC))
			get_int_entry(t += strlen(SEND_RETRY_INTERVAL_USEC), &send_retry_interval_usec);
	}

	return 0;
}

const char *get_default_page() {
	return default_page;
}

const char *get_static_dir() {
	return static_dir;
}

int get_local_port() {
	return local_port;
}

int get_number_of_threads() {
	return threads;
}

const char *get_methods_to_forbid() {
	return methods_to_forbid;
}

int get_send_retry_interval_usec() {
	return send_retry_interval_usec;
}
