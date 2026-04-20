/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * K3CONF Main Header file.
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <socinfo.h>
#include <string.h>

#ifndef __K3CONF_H
#define __K3CONF_H

int process_show_command(int argc, char *argv[]);
int process_dump_command(int argc, char *argv[]);
int dump_clocks_info(int argc, char *argv[]);
int dump_clock_parent_info(int argc, char *argv[]);
int dump_devices_info(int argc, char *argv[]);
int dump_cpu_info(void);
int process_enable_command(int argc, char *argv[]);
int process_disable_command(int argc, char *argv[]);
int process_set_command(int argc, char *argv[]);
int process_read_command(int argc, char *argv[]);
int process_write_command(int argc, char *argv[]);
int ddrbw_info(int argc, char *argv[]);
#endif
