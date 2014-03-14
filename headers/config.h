/*
 * config.h
 *
 *  Created on: Mar 14, 2014
 *      Author: alex
 */

#ifndef CONFIG_H_
#define CONFIG_H_

int init_config();
const char *get_default_page();
const char *get_static_dir();
int get_local_port();
int get_number_of_threads();
const char *get_methods_to_forbid();
int get_send_retry_interval_usec();

#endif /* CONFIG_H_ */
