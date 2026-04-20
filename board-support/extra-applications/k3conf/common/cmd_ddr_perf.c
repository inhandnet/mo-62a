// SPDX-License-Identifier: BSD-3-Clause
/*
 * K3CONF DDR b/w capture
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 *	Aarya Chaumal <a-chaumal@ti.com>
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <socinfo.h>
#include <help.h>
#include <k3conf.h>
#include <mmio.h>
#include <ddr_perf.h>
#include <autoadjust_table.h>

#define TO_USEC(ts)			((ts).tv_sec * 1000000 + (ts).tv_nsec / 1000)
/* A value of 0x00 configures counter 0 to return number of write transactions */
#define PERF_DDR_STATS_CTR0		(0x00)
/* A value of 0x01 configures counter 1 to return number of read transactions */
#define PERF_DDR_STATS_CTR1		(0x01)
#define PERF_CNT_SEL_REG_OFFSET	(0)
#define PERF_CTR0_REG_OFFSET		(4)
#define PERF_CTR1_REG_OFFSET		(8)

struct data_capture_per_inst {
	uint32_t initial_read;
	uint32_t initial_write;
	uint32_t final_read;
	uint32_t final_write;
	struct timespec first_time;
	struct timespec last_time;
};

static inline uintptr_t ctrl_reg_addr(struct ddr_perf_soc_info *pinfo, uint8_t inst)
{
	return pinfo->perf_inst_base[inst] + PERF_CNT_SEL_REG_OFFSET;
}

static inline uintptr_t write_counter_addr(struct ddr_perf_soc_info *pinfo, uint8_t inst)
{
	return pinfo->perf_inst_base[inst] + PERF_CTR0_REG_OFFSET;
}

static inline uintptr_t read_counter_addr(struct ddr_perf_soc_info *pinfo, uint8_t inst)
{
	return pinfo->perf_inst_base[inst] + PERF_CTR1_REG_OFFSET;
}

int ddrbw_info(int argc, char *argv[])
{
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	uint32_t row = 0;
	int ret = 0;
	uint32_t duration = 1;
	int auto_refresh = -1;
	struct ddr_perf_soc_info *pinfo = soc_info.ddr_perf_info;
	struct data_capture_per_inst *dcap;

	if (!pinfo) {
		fprintf(stderr,
			"DDR performance monitoring is not supported on this SoC\n");
		return -1;
	}

	if (argc >= 1) {
		ret = sscanf(argv[0], "%u", &duration);
		if (ret != 1) {
			fprintf(stderr, "Invalid argument for duration.\n");
			help(HELP_DUMP_DDRBW);
			return -1;
		}
	}

	if (argc == 2) {
		ret = sscanf(argv[1], "%d", &auto_refresh);
		if (ret != 1) {
			fprintf(stderr, "Invalid argument for auto_refresh.\n");
			help(HELP_DUMP_DDRBW);
			return -1;
		}
	}

	dcap = calloc(pinfo->num_perf_insts, sizeof(struct data_capture_per_inst));
	if (dcap == NULL) {
		fprintf(stderr, "Unable to allocate capture memory\n");
		return -2;
	}

	/* Set counter 0 and 1 to write and read resp. */
	for (int i = 0; i < pinfo->num_perf_insts; i++) {
		mmio_write_32(ctrl_reg_addr(pinfo, i), PERF_DDR_STATS_CTR1 << 8 |
			      PERF_DDR_STATS_CTR0 << 0);
	}

	autoadjust_table_init(table);
	strncpy(table[row][0], "DDR instance", TABLE_MAX_ELT_LEN);
	strncpy(table[row][1], "Read data (MiB)", TABLE_MAX_ELT_LEN);
	strncpy(table[row][2], "Avg Read B/W (MiB/s)", TABLE_MAX_ELT_LEN);
	strncpy(table[row][3], "Write data (MiB)", TABLE_MAX_ELT_LEN);
	strncpy(table[row][4], "Avg Write B/W (MiB/s)", TABLE_MAX_ELT_LEN);

	while (auto_refresh--) {
		struct timespec timer;

		for (int i = 0; i < pinfo->num_perf_insts; i++) {
			clock_gettime(CLOCK_MONOTONIC, &dcap[i].first_time);
			dcap[i].initial_read = mmio_read_32(read_counter_addr(pinfo, i));
			dcap[i].initial_write =
			    mmio_read_32(write_counter_addr(pinfo, i));
		}

		timer = dcap[pinfo->num_perf_insts - 1].first_time;
		timer.tv_sec += duration;
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timer, NULL);

		for (int i = 0; i < pinfo->num_perf_insts; i++) {
			clock_gettime(CLOCK_MONOTONIC, &dcap[i].last_time);
			dcap[i].final_read = mmio_read_32(read_counter_addr(pinfo, i));
			dcap[i].final_write = mmio_read_32(write_counter_addr(pinfo, i));
		}

		for (int i = 0; i < pinfo->num_perf_insts; i++) {
			uint32_t read_count = dcap[i].final_read - dcap[i].initial_read;
			uint32_t write_count = dcap[i].final_write - dcap[i].initial_write;
			uint64_t time = 0;
			float read_bw = 0, write_bw = 0;

			float read_mibytes = ((float)read_count * pinfo->burst_size) / (1024 * 1024);
			float write_mibytes = ((float)write_count * pinfo->burst_size) / (1024 * 1024);

			time = TO_USEC(dcap[i].last_time) - TO_USEC(dcap[i].first_time);
			read_bw = read_mibytes / (time / 1000000.0f);
			write_bw = write_mibytes / (time / 1000000.0f);

			snprintf(table[i + 1][0], TABLE_MAX_ELT_LEN, "DDR%d", i);
			snprintf(table[i + 1][1], TABLE_MAX_ELT_LEN, "%.3f", read_mibytes);
			snprintf(table[i + 1][2], TABLE_MAX_ELT_LEN, "%.3f", read_bw);
			snprintf(table[i + 1][3], TABLE_MAX_ELT_LEN, "%.3f", write_mibytes);
			snprintf(table[i + 1][4], TABLE_MAX_ELT_LEN, "%.3f", write_bw);
		}
		ret = autoadjust_table_print(table, 1 + pinfo->num_perf_insts, 5);
	}
	free(dcap);

	return ret;
}
