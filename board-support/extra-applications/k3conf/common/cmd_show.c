// SPDX-License-Identifier: BSD-3-Clause
/*
 * K3CONF Command Show
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sec_proxy.h>
#include <tisci.h>
#include <socinfo.h>
#include <help.h>
#include <autoadjust_table.h>
#include <k3conf.h>

static int show_hosts_info(void)
{
	struct ti_sci_host_info *hosts = soc_info.sci_info.host_info;
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	uint32_t row = 0;

	if (soc_info.protocol == SCMI)
		return -1;

	autoadjust_table_init(table);
	strncpy(table[row][0], "Host ID", TABLE_MAX_ELT_LEN);
	strncpy(table[row][1], "Host Name", TABLE_MAX_ELT_LEN);
	strncpy(table[row][2], "Security Status", TABLE_MAX_ELT_LEN);
	strncpy(table[row][3], "Description", TABLE_MAX_ELT_LEN);

	for (row = 0; row < soc_info.sci_info.num_hosts; row++) {
		snprintf(table[row + 1][0], TABLE_MAX_ELT_LEN, "%d",
			 hosts[row].host_id);
		strncpy(table[row + 1][1], hosts[row].host_name,
			TABLE_MAX_ELT_LEN);
		strncpy(table[row + 1][2], hosts[row].security_status,
			TABLE_MAX_ELT_LEN);
		strncpy(table[row + 1][3], hosts[row].description,
			TABLE_MAX_ELT_LEN);
	}

	return autoadjust_table_print(table, row + 1, 4);
}

static int show_sp_threads_info(void)
{
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	struct ti_sci_sec_proxy_info *sp;
	uint32_t row = 0, i;

	if (soc_info.protocol == SCMI)
		return -1;

	autoadjust_table_init(table);
	strncpy(table[row][0],
		"Secure Proxy thread allocation for main_sec_proxy0",
		TABLE_MAX_ELT_LEN);
	row++;
	strncpy(table[row][0], "Thread ID", TABLE_MAX_ELT_LEN);
	strncpy(table[row][1], "Direction", TABLE_MAX_ELT_LEN);
	strncpy(table[row][2], "No. of msgs", TABLE_MAX_ELT_LEN);
	strncpy(table[row][3], "Host", TABLE_MAX_ELT_LEN);
	strncpy(table[row][4], "Host Function", TABLE_MAX_ELT_LEN);
	row++;

	sp = soc_info.sci_info.sp_info[MAIN_SEC_PROXY];
	for (i = 0; i < soc_info.sci_info.num_sp_threads[MAIN_SEC_PROXY];
	     row++, i++) {
		snprintf(table[row][0], TABLE_MAX_ELT_LEN, "%5d", sp[i].sp_id);
		strncpy(table[row][1], sp[i].sp_dir, TABLE_MAX_ELT_LEN);
		snprintf(table[row][2], TABLE_MAX_ELT_LEN, "%7d", sp[i].num_msgs);
		strncpy(table[row][3], sp[i].host, TABLE_MAX_ELT_LEN);
		strncpy(table[row][4], sp[i].host_function, TABLE_MAX_ELT_LEN);
	}

	autoadjust_table_generic_fprint(stdout, table, row, 5,
					TABLE_HAS_SUBTITLE | TABLE_HAS_TITLE);
	row = 0;
	autoadjust_table_init(table);
	strncpy(table[row][0],
		"Secure Proxy thread allocation for mcu_sec_proxy0",
		TABLE_MAX_ELT_LEN);
	row++;
	strncpy(table[row][0], "Thread ID", TABLE_MAX_ELT_LEN);
	strncpy(table[row][1], "Direction", TABLE_MAX_ELT_LEN);
	strncpy(table[row][2], "No. of msgs", TABLE_MAX_ELT_LEN);
	strncpy(table[row][3], "Host", TABLE_MAX_ELT_LEN);
	strncpy(table[row][4], "Host Function", TABLE_MAX_ELT_LEN);
	row++;

	sp = soc_info.sci_info.sp_info[MCU_SEC_PROXY];
	for (i = 0; i < soc_info.sci_info.num_sp_threads[MCU_SEC_PROXY];
	     row++, i++) {
		snprintf(table[row][0], TABLE_MAX_ELT_LEN, "%5d", sp[i].sp_id);
		strncpy(table[row][1], sp[i].sp_dir, TABLE_MAX_ELT_LEN);
		snprintf(table[row][2], TABLE_MAX_ELT_LEN, "%7d", sp[i].num_msgs);
		strncpy(table[row][3], sp[i].host, TABLE_MAX_ELT_LEN);
		strncpy(table[row][4], sp[i].host_function, TABLE_MAX_ELT_LEN);
	}

	return autoadjust_table_generic_fprint(stdout, table, row, 5,
					TABLE_HAS_SUBTITLE | TABLE_HAS_TITLE);
}

static int show_clocks_info(int argc, char *argv[])
{
	struct ti_sci_clocks_info *tisci_c = soc_info.sci_info.clocks_info;
	struct arm_scmi_clocks_info *scmi_c = soc_info.scmi_info.clocks_info;
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	uint32_t row = 0, dev_id, clk_id, num_clks;
	int found = 0, ret;
	const char *clk_name, *clk_function;

	if (soc_info.protocol == TISCI)
		num_clks = soc_info.sci_info.num_clocks;
	else
		num_clks = soc_info.scmi_info.num_clocks;

	autoadjust_table_init(table);
	strncpy(table[row][0], "Device ID", TABLE_MAX_ELT_LEN);
	strncpy(table[row][1], "Clock ID", TABLE_MAX_ELT_LEN);
	strncpy(table[row][2], "Clock Name", TABLE_MAX_ELT_LEN);
	strncpy(table[row][3], "Clock Function", TABLE_MAX_ELT_LEN);

	/* only SoCs using TISCI have the option to print_single_device */
	if (argc == 1 && soc_info.protocol == TISCI)
		goto print_single_device;
	else if (argc > 1)
		goto print_single_clock;

	for (row = 0; row < num_clks; row++) {
		if (soc_info.protocol == TISCI) {
			clk_id = tisci_c[row].clk_id;
			clk_name = tisci_c[row].clk_name;
			clk_function = tisci_c[row].clk_function;
			snprintf(table[row + 1][0], TABLE_MAX_ELT_LEN, "%5d",
				tisci_c[row].dev_id);
		} else {
			clk_id = scmi_c[row].clk_id;
			clk_name = scmi_c[row].clk_name;
			clk_function = scmi_c[row].clk_function;
			strncpy(table[row + 1][0], scmi_c[row].dev_name, TABLE_MAX_ELT_LEN);
		}
		snprintf(table[row + 1][1], TABLE_MAX_ELT_LEN, "%5d",
			clk_id);
		strncpy(table[row + 1][2], clk_name, TABLE_MAX_ELT_LEN);
		strncpy(table[row + 1][3], clk_function, TABLE_MAX_ELT_LEN);
	}

	return autoadjust_table_print(table, row + 1, 4);

print_single_device:
	ret = sscanf(argv[0], "%u", &dev_id);
	if (ret != 1)
		return -1;

	for (row = 0; row < soc_info.sci_info.num_clocks; row++) {
		if (dev_id == tisci_c[row].dev_id) {
			snprintf(table[found + 1][0], TABLE_MAX_ELT_LEN, "%5d",
				 tisci_c[row].dev_id);
			snprintf(table[found + 1][1], TABLE_MAX_ELT_LEN, "%5d",
				 tisci_c[row].clk_id);
			strncpy(table[found + 1][2], tisci_c[row].clk_name,
				TABLE_MAX_ELT_LEN);
			strncpy(table[found + 1][3], tisci_c[row].clk_function, TABLE_MAX_ELT_LEN);
			found++;
		}
	}

	if (!found)
		return -1;

	return autoadjust_table_print(table, found + 1, 4);

print_single_clock:
	ret = sscanf(argv[0], "%u", &dev_id);
	if (ret != 1)
		return -1;

	ret = sscanf(argv[1], "%u", &clk_id);
	if (ret != 1)
		return -1;

	if (soc_info.protocol == TISCI) {
		for (row = 0; row < num_clks; row++) {
			if (dev_id == tisci_c[row].dev_id && clk_id == tisci_c[row].clk_id) {
				clk_name = tisci_c[row].clk_name;
				clk_function = tisci_c[row].clk_function;
				snprintf(table[1][0], TABLE_MAX_ELT_LEN, "%5d",
					tisci_c[row].dev_id);
				found = 1;
				break;
			}
		}
	} else {
		if (clk_id >= num_clks)
			return -1;
		clk_name = scmi_c[clk_id].clk_name;
		clk_function = scmi_c[clk_id].clk_function;
		strncpy(table[1][0], scmi_c[clk_id].dev_name, TABLE_MAX_ELT_LEN);
		found = 1;
	}

	if (!found)
		return -1;

	snprintf(table[1][1], TABLE_MAX_ELT_LEN, "%5d",
		 clk_id);
	strncpy(table[1][2], clk_name,
		TABLE_MAX_ELT_LEN);
	strncpy(table[1][3], clk_function, TABLE_MAX_ELT_LEN);

	return autoadjust_table_print(table, 2, 4);
}

static int show_devices_info(int argc, char *argv[])
{
	struct ti_sci_devices_info *tisci_p = soc_info.sci_info.devices_info;
	struct arm_scmi_devices_info *scmi_p = soc_info.scmi_info.devices_info;
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	uint32_t row = 0, dev_id, num_devices;
	int found = 0, ret;
	const char *dev_name;

	if (soc_info.protocol == TISCI)
		num_devices = soc_info.sci_info.num_devices;
	else
		num_devices = soc_info.scmi_info.num_devices;

	autoadjust_table_init(table);
	strncpy(table[row][0], "Device ID", TABLE_MAX_ELT_LEN);
	strncpy(table[row][1], "Device Name", TABLE_MAX_ELT_LEN);

	if (argc)
		goto print_single_device;

	for (row = 0; row < num_devices; row++) {
		if (soc_info.protocol == TISCI) {
			dev_id = tisci_p[row].dev_id;
			dev_name = tisci_p[row].name;
		} else {
			dev_id = scmi_p[row].dev_id;
			dev_name = scmi_p[row].name;
		}
		snprintf(table[row + 1][0], TABLE_MAX_ELT_LEN, "%5d",
			dev_id);
		strncpy(table[row + 1][1], dev_name, TABLE_MAX_ELT_LEN);
	}

	return autoadjust_table_print(table, row + 1, 2);

print_single_device:
	ret = sscanf(argv[0], "%u", &dev_id);
	if (ret != 1)
		return -1;

	if (soc_info.protocol == TISCI) {
		for (row = 0; row < soc_info.sci_info.num_devices; row++) {
			if (dev_id == tisci_p[row].dev_id) {
				snprintf(table[1][0], TABLE_MAX_ELT_LEN, "%5d",
					tisci_p[row].dev_id);
				strncpy(table[1][1], tisci_p[row].name,
					TABLE_MAX_ELT_LEN);
				found = 1;
				break;
			}
		}
	} else {
		if (dev_id >= num_devices)
			return -1;
		snprintf(table[1][0], TABLE_MAX_ELT_LEN, "%5d", dev_id);
		strncpy(table[1][1], scmi_p[dev_id].name, TABLE_MAX_ELT_LEN);
		found = 1;
	}

	if (!found)
		return -1;

	return autoadjust_table_print(table, 2, 2);
}

static int show_rm_info(int argc, char *argv[])
{
	struct ti_sci_devices_info *d = soc_info.sci_info.devices_info;
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	struct ti_sci_rm_info *r = soc_info.sci_info.rm_info;
	uint32_t filter_type = 0, type, subtype, i, j, row;
	char *subtype_name;

	if (soc_info.protocol == SCMI)
		return -1;

	if (argc > 1)
		return -1;
	if (argc == 1)
		sscanf(argv[0], "%u", &filter_type);

	snprintf(table[0][0], TABLE_MAX_ELT_LEN,
			"Resources managed by System Firmware");
	snprintf(table[1][0], TABLE_MAX_ELT_LEN, "Unique Type");
	snprintf(table[1][1], TABLE_MAX_ELT_LEN, "dev_id");
	snprintf(table[1][2], TABLE_MAX_ELT_LEN, "Device name");
	snprintf(table[1][3], TABLE_MAX_ELT_LEN, "subtype_id");
	snprintf(table[1][4], TABLE_MAX_ELT_LEN, "Subtype name");

	row = 2;
	for (i = 0; i < soc_info.sci_info.num_res; i++) {

		type = r[i].utype >> 6;
		subtype = r[i].utype & 0x3F;
		subtype_name = r[i].subtype_name;

		if (filter_type && filter_type != type)
			continue;

		for (j = 0; j < soc_info.sci_info.num_devices; j++) {
			if (type == d[j].dev_id)
				break;
		}

		snprintf(table[row][0], TABLE_MAX_ELT_LEN,
			"0x%04x", r[i].utype);
		snprintf(table[row][1], TABLE_MAX_ELT_LEN,
			"%d", d[j].dev_id);
		snprintf(table[row][2], TABLE_MAX_ELT_LEN,
			"%s", d[j].name);
		snprintf(table[row][3], TABLE_MAX_ELT_LEN,
			"%d", subtype);
		snprintf(table[row][4], TABLE_MAX_ELT_LEN,
			"%s", subtype_name);
		row++;
	}

	if (row == 2) {
		fprintf(stderr, "Resources for type %d are not managed by SYSFW\n",
				filter_type);
		return -1;
	}

	return autoadjust_table_generic_fprint(stdout, table, row, 5,
			TABLE_HAS_SUBTITLE | TABLE_HAS_TITLE);
}

static int show_processors_info(void)
{
	struct ti_sci_processors_info *tisci_p = soc_info.sci_info.processors_info;
	struct arm_scmi_processors_info *scmi_p = soc_info.scmi_info.processors_info;
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	uint32_t row = 0, num_procs;

	if (soc_info.protocol == TISCI)
		num_procs = soc_info.sci_info.num_processors;
	else
		num_procs = soc_info.scmi_info.num_processors;

	autoadjust_table_init(table);
	strncpy(table[row][0], "Device ID", TABLE_MAX_ELT_LEN);
	strncpy(table[row][1], "Processor ID", TABLE_MAX_ELT_LEN);
	strncpy(table[row][2], "Processor Name", TABLE_MAX_ELT_LEN);

	for (row = 0; row < num_procs; row++) {
		if (soc_info.protocol == TISCI) {
			snprintf(table[row + 1][0], TABLE_MAX_ELT_LEN, "%5d",
				tisci_p[row].dev_id);
			snprintf(table[row + 1][1], TABLE_MAX_ELT_LEN, "%7d",
				tisci_p[row].processor_id);
			strncpy(table[row + 1][2], tisci_p[row].name, TABLE_MAX_ELT_LEN);
		}
		else {
			snprintf(table[row + 1][0], TABLE_MAX_ELT_LEN, "%s",
				"SCMI: NA");
			snprintf(table[row + 1][1], TABLE_MAX_ELT_LEN, "%s",
				"SCMI: NOT SUPPORTED");
			strncpy(table[row + 1][2], scmi_p[row].dev_name, TABLE_MAX_ELT_LEN);
		}
	}

	return autoadjust_table_print(table, row + 1, 3);
}

static int show_msmc_info(void)
{
	uint32_t s_h, s_l, e_h, e_l;
	int ret;
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];

	if (soc_info.protocol == SCMI)
		return -1;

	ret = ti_sci_cmd_get_msmc(&s_l, &s_h, &e_l, &e_h);
	if (ret)
		return ret;

	autoadjust_table_init(table);
	strncpy(table[0][0], "MSMC_START_ADDRESS", TABLE_MAX_ELT_LEN);
	strncpy(table[0][1], "MSMC_END_ADDRESS", TABLE_MAX_ELT_LEN);

	snprintf(table[1][0], TABLE_MAX_ELT_LEN, "0x%08x%08x", s_h, s_l);
	snprintf(table[1][1], TABLE_MAX_ELT_LEN, "0x%08x%08x", e_h, e_l);
	return autoadjust_table_print(table, 2, 2);
}

int process_show_command(int argc, char *argv[])
{
	int ret;

	if (argc < 1) {
		help(HELP_SHOW);
		return -1;
	}

	if (!strncmp(argv[0], "host", 4)) {
		ret = show_hosts_info();
		if (ret) {
			if (soc_info.protocol == SCMI)
				fprintf(stderr, "SCMI_ERROR: %s %d\n", scmi_status_code[-ret], ret);
			else
				help(HELP_SHOW_HOST);
		}
	} else if (!strncmp(argv[0], "thread", 6)) {
		ret = show_sp_threads_info();
		if (ret) {
			if (soc_info.protocol == SCMI)
				fprintf(stderr, "SCMI_ERROR: %s %d\n", scmi_status_code[-ret], ret);
			else
				help(HELP_SHOW_SEC_PROXY);
		}
	} else if (!strncmp(argv[0], "device", 6)) {
		argc--;
		argv++;
		ret = show_devices_info(argc, argv);
		if (ret) {
			fprintf(stderr, "Invalid device arguments\n");
			help(HELP_SHOW_DEVICE);
		}
	} else if (!strncmp(argv[0], "clock", 5)) {
		argc--;
		argv++;
		ret = show_clocks_info(argc, argv);
		if (ret) {
			fprintf(stderr, "Invalid clock arguments\n");
			help(HELP_SHOW_CLOCK);
		}
	} else if(!strncmp(argv[0], "processor", 9)) {
		ret = show_processors_info();
		if (ret)
			help(HELP_SHOW_PROCESSOR);
	} else if(!strncmp(argv[0], "rm", 2)) {
		argc--;
		argv++;
		ret = show_rm_info(argc, argv);
		if (ret) {
			if (soc_info.protocol == SCMI)
				fprintf(stderr, "SCMI_ERROR: %s %d\n", scmi_status_code[-ret], ret);
			else {
				fprintf(stderr, "Invalid device_id arguments\n");
				help(HELP_SHOW_RM);
			}
		}
	} else if (!strncmp(argv[0], "msmc", 4)) {
		ret = show_msmc_info();
		if (ret) {
			if (soc_info.protocol == SCMI)
				fprintf(stderr, "SCMI_ERROR: %s %d\n", scmi_status_code[-ret], ret);
			else
				help(HELP_SHOW_MSMC);
		}
	} else if (!strcmp(argv[0], "--help")) {
		help(HELP_SHOW);
		return 0;
	} else {
		fprintf(stderr, "Invalid argument %s\n", argv[1]);
		help(HELP_SHOW);
		return -1;
	}
	return ret;
}
