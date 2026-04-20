/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * SCMI helper apis header file
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SCMI Specification: https://developer.arm.com/documentation/den0056/latest/
 */

#ifndef __SCMI_H
#define __SCMI_H

#include <stdint.h>

#define MAX_PROC_NAME_LEN	30
#define MAX_DEV_NAME_LEN	60
#define MAX_CLK_NAME_LEN	120
#define MAX_CLK_FUNC_LEN	30

struct arm_scmi_processors_info {
	char dev_name[MAX_PROC_NAME_LEN];
	uint32_t clk_id;
};

struct arm_scmi_devices_info {
	uint32_t dev_id;
	char name[MAX_DEV_NAME_LEN];
};

struct arm_scmi_clocks_info {
	char dev_name[MAX_DEV_NAME_LEN];
	uint32_t clk_id;
	char clk_name[MAX_CLK_NAME_LEN];
	char clk_function[MAX_CLK_FUNC_LEN];
};

struct arm_scmi_version_info {
	uint32_t impl_version;
};

struct arm_scmi_raw_interface_info {
	uint32_t max_rx_timeout_ms;
};

struct arm_scmi_info {
	struct arm_scmi_version_info version;
	struct arm_scmi_processors_info *processors_info;
	uint32_t num_processors;
	struct arm_scmi_devices_info *devices_info;
	uint32_t num_devices;
	struct arm_scmi_clocks_info *clocks_info;
	uint32_t num_clocks;
};

#define SCMI_MAX_POS_PARENT_CLKS	20

int scmi_init(void);
const char *scmi_cmd_get_device_status(uint32_t dev_id);
int scmi_cmd_disable_device(uint32_t dev_id);
int scmi_cmd_enable_device(uint32_t dev_id);
int scmi_cmd_enable_clk(uint32_t clk_id);
int scmi_cmd_disable_clk(uint32_t clk_id);
const char *scmi_cmd_get_clk_state(uint32_t clk_id, uint32_t flags);
int scmi_cmd_set_clk_freq(uint32_t clk_id, uint64_t freq, uint32_t flags);
int scmi_cmd_get_clk_freq(uint32_t clk_id, uint64_t *freq);
int scmi_cmd_get_clk_parent(uint32_t clk_id, uint32_t *parent_clk_id);
int scmi_cmd_set_clk_parent(uint32_t clk_id, uint32_t parent_clk_id);
int scmi_cmd_get_clk_possible_parents(uint32_t clk_id, uint32_t *num_parents, uint32_t *parent_arr);

#define SCMI_MAX_STAT_CODE_LEN	50
#define SCMI_NUM_STAT_CODES	13

static const char scmi_status_code[SCMI_NUM_STAT_CODES][SCMI_MAX_STAT_CODE_LEN] = {
	"SUCCESS",
	"NOT_SUPPORTED",
	"INVALID_PARAMETERS",
	"DENIED",
	"NOT_FOUND",
	"OUT_OF_RANGE",
	"BUSY",
	"COMMS_ERROR",
	"GENERIC_ERROR",
	"HARDWARE_ERROR",
	"PROTOCOL_ERROR",
	"IN_USE",
	"PARTIAL_ERROR"
};

#endif
