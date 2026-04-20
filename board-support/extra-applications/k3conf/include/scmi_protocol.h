/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * SCMI Protocol header file
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SCMI Specification: https://developer.arm.com/documentation/den0056/latest/
 */

#ifndef __SCMI_P_H
#define __SCMI_P_H

#include <stdint.h>
#include <scmi_raw_interface.h>

/* Protocol IDs */
#define SCMI_BASE_ID	0x10
#define SCMI_PWR_DOMAIN_ID	0x11
#define SCMI_CLK_ID	0x14

/* Message IDs */
#define SCMI_GET_IMPL_VERS_MSG_ID	0x05

/* Power Domain */
#define SCMI_SET_PWR_DOMAIN_STATE_MSG_ID	0x04
#define SCMI_GET_PWR_DOMAIN_STATE_MSG_ID	0x05

/* Clock */
#define SCMI_SET_CLK_FREQ_MSG_ID	0x05
#define SCMI_GET_CLK_FREQ_MSG_ID	0x06
#define SCMI_SET_CLK_STATE_MSD_ID	0x07
#define SCMI_GET_CLK_STATE_MSG_ID	0x0B
#define SCMI_GET_CLK_POS_PARENTS_MSG_ID	0x0C
#define SCMI_SET_CLK_PARENT_MSG_ID	0x0D
#define SCMI_GET_CLK_PARENT_MSG_ID	0x0E

/* Message Types */
#define SCMI_CMD_MSG_TYPE	0x00

#define SCMI_DEVICE_STATE_ON	0x00000000
#define SCMI_DEVICE_STATE_OFF	0x40000000

#define SCMI_CLOCK_STATE_DISABLED	0
#define SCMI_CLOCK_STATE_ENABLED	1

struct scmi_msg_hdr {
	uint32_t hdr;
} __attribute__ ((__packed__));

struct scmi_msg_resp_impl_version {
	struct scmi_msg_hdr hdr;
	int32_t status;
	uint32_t impl_version;
} __attribute__ ((__packed__));

struct scmi_msg_req_get_device_state {
	struct scmi_msg_hdr hdr;
	uint32_t id;
} __attribute__ ((__packed__));

struct scmi_msg_resp_get_device_state {
	struct scmi_msg_hdr hdr;
	int32_t status;
	uint32_t power_state;
} __attribute__ ((__packed__));

struct scmi_msg_req_set_pwr_domain_state {
	struct scmi_msg_hdr hdr;
	uint32_t flags;
	uint32_t id;
	uint32_t power_state;
} __attribute__ ((__packed__));

struct scmi_msg_req_set_clk_state {
	struct scmi_msg_hdr hdr;
	uint32_t id;
	uint32_t attributes;
	uint32_t ext_config_val;
} __attribute__ ((__packed__));

struct scmi_msg_req_get_clk_state {
	struct scmi_msg_hdr hdr;
	uint32_t id;
	uint32_t flags;
} __attribute__ ((__packed__));

struct scmi_msg_resp_get_clk_state {
	struct scmi_msg_hdr hdr;
	int32_t status;
	uint32_t attributes;
	uint32_t config;
	uint32_t extended_config_val;
} __attribute__ ((__packed__));

struct scmi_msg_req_set_clk_freq {
	struct scmi_msg_hdr hdr;
	uint32_t flags;
	uint32_t id;
	uint64_t rate;
} __attribute__ ((__packed__));

struct scmi_msg_req_get_clk_freq {
	struct scmi_msg_hdr hdr;
	uint32_t id;
} __attribute__ ((__packed__));

struct scmi_msg_resp_get_clk_freq {
	struct scmi_msg_hdr hdr;
	int32_t status;
	uint64_t rate;
} __attribute__ ((__packed__));

struct scmi_msg_req_get_clk_parent {
	struct scmi_msg_hdr hdr;
	uint32_t clk_id;
} __attribute__ ((__packed__));

struct scmi_msg_resp_get_clk_parent {
	struct scmi_msg_hdr hdr;
	int32_t status;
	uint32_t parent_id;
} __attribute__((__packed__));

struct scmi_msg_req_set_clk_parent {
	struct scmi_msg_hdr hdr;
	uint32_t clk_id;
	uint32_t parent_id;
} __attribute__ ((__packed__));

struct scmi_msg_req_get_clk_pos_parents {
	struct scmi_msg_hdr hdr;
	uint32_t clk_id;
	uint32_t skip_parents;
} __attribute__((__packed__));

struct scmi_msg_resp_get_clk_pos_parents {
	struct scmi_msg_hdr hdr;
	int32_t status;
	uint32_t flags;
	uint32_t *possible_parents;
} __attribute__((__packed__));

void scmi_setup_header(struct scmi_msg_hdr *msg_hdr, uint32_t protocol_id, uint32_t msg_id,
			 uint32_t msg_type);
int scmi_xfer_msg(struct scmi_raw_msg *msg);

#endif
