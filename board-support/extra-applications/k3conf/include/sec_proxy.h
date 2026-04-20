/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Secure Proxy header file
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <stdio.h>
#include <stdint.h>

#ifndef __SEC_PROXY_H
#define __SEC_PROXY_H

#define DEFAULT_HOST_ID		14
#define INVALID_HOST_ID		0xff
#define DEFAULT_SEC_PROXY_RX_THREAD	21
#define DEFAULT_SEC_PROXY_TX_THREAD	23

#define SEC_PROXY_MAX_MSG_SIZE	60
#define SEC_PROXY_HOST		14

struct k3_sec_proxy_msg {
	size_t len;
	uint8_t *buf;
};

struct k3_sec_proxy_base {
	uint32_t src_target_data;
	uint32_t cfg_scfg;
	uint32_t cfg_rt;
};

int k3_sec_proxy_send(struct k3_sec_proxy_msg *msg);
int k3_sec_proxy_recv(struct k3_sec_proxy_msg *msg);
int k3_sec_proxy_init(void);

extern struct k3_sec_proxy_base k3_generic_sec_proxy_base;
extern struct k3_sec_proxy_base k3_lite_sec_proxy_base;

#endif /* __SEC_PROXY_H */
