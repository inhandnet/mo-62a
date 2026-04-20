/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * SoC info header file
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef __SOCINFO_H
#define __SOCINFO_H

#include <stdint.h>
#include <tisci.h>
#include <scmi.h>
#include <autoadjust_table.h>
#include "sec_proxy.h"
#include "ddr_perf.h"

#define AM62X	0xbb7e
#define AM62AX	0xbb8d
#define AM62PX	0xbb9d
#define AM62LX	0xbba7
#define AM65X	0xbb5a
#define J721E	0xbb64
#define J7200	0xbb6d
#define AM64X	0xbb38
#define J721S2	0xbb75
#define J722S	0xbba0
#define J784S4	0xbb80

#define DEVICE_ID_PKG_J784S4 0x5
#define DEVICE_ID_PKG_J742S2 0x7

typedef enum {
	TISCI,
	SCMI,
} comm_protocol;

struct k3conf_soc_info {
	const char *soc_name;
	const char *rev_name;
	char dev_part_identifier[TABLE_MAX_ELT_LEN];
	char die_id[TABLE_MAX_ELT_LEN];
	uint8_t host_id;
	uint8_t ti_sci_enabled;
	uint8_t scmi_enabled;
	comm_protocol protocol;
	struct ti_sci_info sci_info;
	struct arm_scmi_info scmi_info;
	struct ddr_perf_soc_info *ddr_perf_info;
	struct k3_sec_proxy_base *sec_proxy;
};

extern struct k3conf_soc_info soc_info;

int soc_init(uint32_t host_id);

extern int soc_info_valid;

#define SOC_INFO_UNKNOWN_SILICON (-19)

#endif
