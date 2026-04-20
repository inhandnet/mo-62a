// SPDX-License-Identifier: BSD-3-Clause
/*
 * K3 Secure proxy driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <sec_proxy.h>
#include <mmio.h>
#include <socinfo.h>
#include <string.h>

#ifdef DEBUG
#define dprintf(format, ...)	 printf(format, ## __VA_ARGS__)
#else
#define dprintf(format, ...)
#endif

/* SEC PROXY RT THREAD STATUS */
#define RT_THREAD_STATUS			0x0
#define RT_THREAD_THRESHOLD			0x4
#define RT_THREAD_STATUS_ERROR_SHIFT		31
#define RT_THREAD_STATUS_ERROR_MASK		(1 << 31)
#define RT_THREAD_STATUS_CUR_CNT_SHIFT		0
#define RT_THREAD_STATUS_CUR_CNT_MASK		0xff

/* SEC PROXY SCFG THREAD CTRL */
#define SCFG_THREAD_CTRL			0x1000
#define SCFG_THREAD_CTRL_DIR_SHIFT		31
#define SCFG_THREAD_CTRL_DIR_MASK		(1 << 31)

#define SEC_PROXY_THREAD(base, x)		((base) + (0x1000 * (x)))
#define SEC_PROXY_TX_THREAD			0
#define SEC_PROXY_RX_THREAD			1
#define SEC_PROXY_MAX_THREADS			2

#define SEC_PROXY_TIMEOUT_US			1000000

#define SEC_PROXY_DATA_START_OFFS		0x4
#define SEC_PROXY_DATA_END_OFFS			0x3c

struct k3_sec_proxy_base k3_generic_sec_proxy_base = {
	.src_target_data = 0x32c00000,
	.cfg_scfg = 0x32800000,
	.cfg_rt = 0x32400000,
};

struct k3_sec_proxy_base k3_lite_sec_proxy_base = {
	.src_target_data = 0x4d000000,
	.cfg_scfg = 0x4a400000,
	.cfg_rt = 0x4a600000,
};

struct k3_sec_proxy_thread {
	uint32_t id;
	uintptr_t data;
	uintptr_t scfg;
	uintptr_t rt;
} spts[SEC_PROXY_MAX_THREADS];

static inline uint32_t sp_readl(uintptr_t addr)
{
	return mmio_read_32(addr);
}

static inline void sp_writel(uintptr_t addr, uint32_t data)
{
	mmio_write_32(addr, data);
}

static int k3_sec_proxy_verify_thread(uint32_t dir)
{
	struct k3_sec_proxy_thread *spt = &spts[dir];

	/* Check for any errors already available */
	if (sp_readl(spt->rt + RT_THREAD_STATUS) &
	    RT_THREAD_STATUS_ERROR_MASK) {
		fprintf(stderr, "%s: Thread %d is corrupted, cannot send data.\n",
		       __func__, spt->id);
		return -1;
	}

	/* Make sure thread is configured for right direction */
	if ((sp_readl(spt->scfg + SCFG_THREAD_CTRL)
	    & SCFG_THREAD_CTRL_DIR_MASK) >> SCFG_THREAD_CTRL_DIR_SHIFT != dir) {
		if (dir)
			fprintf(stderr, "%s: Trying to receive data on tx Thread %d\n",
			       __func__, spt->id);
		else
			fprintf(stderr, "%s: Trying to send data on rx Thread %d\n",
			       __func__, spt->id);
		return -1;
	}

	/* Check the message queue before sending/receiving data */
	if (!(sp_readl(spt->rt + RT_THREAD_STATUS) &
	      RT_THREAD_STATUS_CUR_CNT_MASK))
		return -2;

	return 0;
}

int k3_sec_proxy_send(struct k3_sec_proxy_msg *msg)
{
	struct k3_sec_proxy_thread *spt = &spts[SEC_PROXY_TX_THREAD];
	int num_words, trail_bytes, ret;
	uint32_t *word_data;
	uintptr_t data_reg;

	ret = k3_sec_proxy_verify_thread(SEC_PROXY_TX_THREAD);
	if (ret) {
		fprintf(stderr, "%s: Thread%d verification failed. ret = %d\n",
			__func__, spt->id, ret);
		return ret;
	}

	/* Check the message size. */
	if (msg->len > SEC_PROXY_MAX_MSG_SIZE) {
		fprintf(stderr, "%s: Thread %u message length %zu > max msg size %d\n",
		       __func__, spt->id, msg->len, SEC_PROXY_MAX_MSG_SIZE);
		return -1;
	}

	/* Send the message */
	data_reg = spt->data + SEC_PROXY_DATA_START_OFFS;
	word_data = (uint32_t *)msg->buf;
	for (num_words = msg->len / sizeof(uint32_t);
	     num_words;
	     num_words--, data_reg += sizeof(uint32_t), word_data++)
		sp_writel(data_reg, *word_data);

	trail_bytes = msg->len % sizeof(uint32_t);
	if (trail_bytes) {
		uint32_t data_trail = *word_data;

		/* Ensure all unused data is 0 */
		data_trail &= 0xFFFFFFFF >> (8 * (sizeof(uint32_t) - trail_bytes));
		sp_writel(data_reg, data_trail);
		data_reg++;
	}

	/*
	 * 'data_reg' indicates next register to write. If we did not already
	 * write on tx complete reg(last reg), we must do so for transmit
	 */
	if (data_reg <= (spt->data + SEC_PROXY_DATA_END_OFFS))
		sp_writel(spt->data + SEC_PROXY_DATA_END_OFFS, 0);

	return 0;
}

int k3_sec_proxy_recv(struct k3_sec_proxy_msg *msg)
{
	struct k3_sec_proxy_thread *spt = &spts[SEC_PROXY_RX_THREAD];
	int num_words, ret = -1, retry = 10000;
	uint32_t *word_data;
	uintptr_t data_reg;

	while (retry-- && ret) {
		ret = k3_sec_proxy_verify_thread(SEC_PROXY_RX_THREAD);
		if ((ret && ret != -2) || !retry) {
			fprintf(stderr, "%s: Thread%d verification failed. ret = %d\n",
				__func__, spt->id, ret);
			return ret;
		}
	}

	data_reg = spt->data + SEC_PROXY_DATA_START_OFFS;
	word_data = (uint32_t *)(uintptr_t)msg->buf;
	for (num_words = SEC_PROXY_MAX_MSG_SIZE / sizeof(uint32_t);
	     num_words;
	     num_words--, data_reg += sizeof(uint32_t), word_data++)
		*word_data = sp_readl(data_reg);

	return 0;
}

static int get_thread_id(char *host_name, char *function)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;
	uint32_t i;

	for (i = 0; i < sci_info->num_sp_threads[MAIN_SEC_PROXY]; i++)
		if (!strcmp(host_name,
			    sci_info->sp_info[MAIN_SEC_PROXY][i].host) &&
		    !strcmp(function,
			    sci_info->sp_info[MAIN_SEC_PROXY][i].host_function))
			return sci_info->sp_info[MAIN_SEC_PROXY][i].sp_id;

	return -1;

}

static char* get_host_name(uint32_t host_id)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;
	uint32_t i;

	for (i = 0; i < sci_info->num_hosts; i++)
		if (host_id == sci_info->host_info[i].host_id)
			return sci_info->host_info[i].host_name;

	return NULL;
}

int k3_sec_proxy_init(void)
{
	struct k3_sec_proxy_base *spb = soc_info.sec_proxy;
	int rx_thread, tx_thread;
	char *host_name;

	host_name = get_host_name(soc_info.host_id);
	if (!host_name) {
		fprintf(stderr, "Invalid host id %d, using default host_id %d\n",
			soc_info.host_id, DEFAULT_HOST_ID);
		soc_info.host_id = DEFAULT_HOST_ID;
		host_name = get_host_name(soc_info.host_id);
	}

	rx_thread = get_thread_id(host_name, "response");
	if (rx_thread < 0) {
		fprintf(stderr, "Invalid host id %d, using default host_id %d\n",
			soc_info.host_id, DEFAULT_HOST_ID);
		soc_info.host_id = DEFAULT_HOST_ID;
		host_name = get_host_name(soc_info.host_id);
		rx_thread = get_thread_id(host_name, "response");
	}
	tx_thread = get_thread_id(host_name, "low_priority");
	dprintf("host_name = %s, tx_thread = %d, rx_thread = %d\n",
		host_name, tx_thread, rx_thread);

	spts[SEC_PROXY_TX_THREAD].id = tx_thread;
	spts[SEC_PROXY_TX_THREAD].data = SEC_PROXY_THREAD(spb->src_target_data, tx_thread);
	spts[SEC_PROXY_TX_THREAD].scfg = SEC_PROXY_THREAD(spb->cfg_scfg, tx_thread);
	spts[SEC_PROXY_TX_THREAD].rt = SEC_PROXY_THREAD(spb->cfg_rt, tx_thread);

	spts[SEC_PROXY_RX_THREAD].id = rx_thread;
	spts[SEC_PROXY_RX_THREAD].data = SEC_PROXY_THREAD(spb->src_target_data, rx_thread);
	spts[SEC_PROXY_RX_THREAD].scfg = SEC_PROXY_THREAD(spb->cfg_scfg, rx_thread);
	spts[SEC_PROXY_RX_THREAD].rt = SEC_PROXY_THREAD(spb->cfg_rt, rx_thread);

	return 0;
}
