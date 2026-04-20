/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * TISCI helper apis header file
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef __TISCI_H
#define __TISCI_H

#include <stdint.h>

struct ti_sci_caps_info {
	uint8_t valid;
	uint64_t fw_caps;
};

struct ti_sci_dm_version_info {
	uint8_t valid;
	uint16_t dm_version;
	uint8_t sub_version;
	uint8_t patch_version;
	uint8_t abi_major;
	uint8_t abi_minor;
	char rm_pm_hal_version[12];
	char sci_server_version[26];
};

struct ti_sci_version_info {
	uint8_t abi_major;
	uint8_t abi_minor;
	uint16_t firmware_version;
	char firmware_description[32];
	struct ti_sci_caps_info caps_info;
	struct ti_sci_dm_version_info dm_info;
};

struct ti_sci_host_info {
	uint32_t host_id;
	char host_name[15];
	char security_status[15];
	char description[50];
};

#define MAIN_SEC_PROXY	0
#define MCU_SEC_PROXY	1

struct ti_sci_sec_proxy_info {
	uint32_t sp_id;
	char sp_dir[6];
	uint32_t num_msgs;
	char host[15];
	char host_function[50];
};

struct ti_sci_processors_info {
	uint32_t dev_id;
	uint32_t clk_id;
	uint32_t processor_id;
	char name[30];
};

struct ti_sci_devices_info {
	uint32_t dev_id;
	char name[60];
};

struct ti_sci_clocks_info {
	uint32_t dev_id;
	uint32_t clk_id;
	char clk_name[100];
	char clk_function[100];
};

struct ti_sci_rm_info {
	uint32_t utype;
	char subtype_name[100];
};

struct ti_sci_info {
	uint8_t host_id;
	struct ti_sci_version_info version;
	struct ti_sci_host_info *host_info;
	uint32_t num_hosts;
	struct ti_sci_sec_proxy_info *sp_info[2];
	uint32_t num_sp_threads[2];
	struct ti_sci_processors_info *processors_info;
	uint32_t num_processors;
	struct ti_sci_devices_info *devices_info;
	uint32_t num_devices;
	struct ti_sci_clocks_info *clocks_info;
	uint32_t num_clocks;
	struct ti_sci_rm_info *rm_info;
	uint32_t num_res;
};

struct ti_sci_rm_desc {
	uint16_t start;
	uint16_t num;
	uint16_t start_sec;
	uint16_t num_sec;
};

#define MAX_DEVICE_STATE_LENGTH		25
#define MAX_CLOCK_STATE_LENGTH		25

int ti_sci_init(void);
int ti_sci_cmd_get_msmc(uint32_t *s_l, uint32_t *s_h,
			uint32_t *e_l, uint32_t *e_h);
const char *ti_sci_cmd_get_device_status(uint32_t dev_id);
int ti_sci_cmd_disable_device(uint32_t dev_id);
int ti_sci_cmd_enable_device(uint32_t dev_id);

int ti_sci_cmd_get_clk(uint32_t dev_id, uint32_t clk_id);
int ti_sci_cmd_put_clk(uint32_t dev_id, uint32_t clk_id);
const char *ti_sci_cmd_get_clk_state(uint32_t dev_id, uint32_t clk_id);
int ti_sci_cmd_set_clk_freq(uint32_t dev_id, uint32_t clk_id, uint64_t freq);
int ti_sci_cmd_get_clk_freq(uint32_t dev_id, uint32_t clk_id, uint64_t *freq);
int ti_sci_cmd_get_range(uint16_t type, uint16_t subtype, uint16_t host_id,
				struct ti_sci_rm_desc *desc);
int ti_sci_cmd_get_clk_parent(uint32_t dev_id, uint32_t clk_id,
			      uint32_t *parent_clk_id);
int ti_sci_cmd_set_clk_parent(uint32_t dev_id, uint32_t clk_id,
			      uint32_t parent_clk_id);
#endif
