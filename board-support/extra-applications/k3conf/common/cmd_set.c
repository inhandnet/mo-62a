// SPDX-License-Identifier: BSD-3-Clause
/*
 * K3CONF Command Set
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <tisci.h>
#include <socinfo.h>
#include <help.h>
#include <k3conf.h>
#include <inttypes.h>

#define HELP_SET_PARENT_URL1 "http://downloads.ti.com/tisci/esd/latest/2_tisci_msgs/pm/clocks.html#power-management-clock-frequency-configuration-example-with-mux-programming"
#define HELP_SET_PARENT_URL2 "http://downloads.ti.com/tisci/esd/latest/2_tisci_msgs/pm/clocks.html#tisci-msg-set-clock-parent"

static int set_clock(int argc, char *argv[])
{
	uint32_t dev_id, clk_id, ret;
	uint64_t freq;

	if (argc < 3)
		return -1;

	ret = sscanf(argv[0], "%u", &dev_id);
	if (ret != 1)
		return -1;

	ret = sscanf(argv[1], "%u", &clk_id);
	if (ret != 1)
		return -1;

	ret = sscanf(argv[2], "%" PRIu64, &freq);
	if (ret != 1)
		return -1;

	if (soc_info.protocol == TISCI)
		ret = ti_sci_cmd_set_clk_freq(dev_id, clk_id, freq);
	else
		/* dev_id is ignored for SoCs using SCMI */
		ret = scmi_cmd_set_clk_freq(clk_id, freq, 0);

	if (ret)
		return ret;

	return dump_clocks_info(1, argv);
}

static int set_clock_parent(int argc, char *argv[])
{
	uint32_t dev_id, clk_id, parent_clk_id;
	int ret;

	if (argc < 3)
		return -1;

	ret = sscanf(argv[0], "%u", &dev_id);
	if (ret != 1)
		return -1;

	ret = sscanf(argv[1], "%u", &clk_id);
	if (ret != 1)
		return -1;

	ret = sscanf(argv[2], "%u", &parent_clk_id);
	if (ret != 1)
		return -1;

	if (soc_info.protocol == TISCI)
		ret = ti_sci_cmd_set_clk_parent(dev_id, clk_id, parent_clk_id);
	else {
		/* dev_id is ignored for SoCs using SCMI */
		ret = scmi_cmd_set_clk_parent(clk_id, parent_clk_id);
	}
	if (ret) {
		fprintf(stderr, "Request to set parent failed: %d\n",ret);
		fprintf(stderr, "Clock state is probably wrong!\n");
		if (soc_info.protocol == TISCI) {
			fprintf(stderr, "Clock state of clk_id %d: %s\n",
				clk_id, ti_sci_cmd_get_clk_state(dev_id, clk_id));
			fprintf(stderr, "Clock state of parent_clk_id %d: %s\n",
				parent_clk_id,
				ti_sci_cmd_get_clk_state(dev_id, parent_clk_id));
		} else {
			fprintf(stderr, "Clock state of clk_id %d: %s\n",
				clk_id, scmi_cmd_get_clk_state(clk_id, 0));
			fprintf(stderr, "Clock state of parent_clk_id %d: %s\n",
				parent_clk_id, scmi_cmd_get_clk_state(parent_clk_id, 0));
		}
		fprintf(stderr, "\nRefer to:\n\t%s\n\t%s\n",
			HELP_SET_PARENT_URL1, HELP_SET_PARENT_URL2);

		return ret;
	}

	return dump_clock_parent_info(argc - 1, argv);
}

int process_set_command(int argc, char *argv[])
{
	int ret;

	if (argc < 1) {
		help(HELP_SET);
		return -1;
	}

	if (!strncmp(argv[0], "clock", 5)) {
		argc--;
		argv++;
		ret = set_clock(argc, argv);
		if (ret) {
			if (soc_info.protocol == SCMI)
				fprintf(stderr, "SCMI_ERROR: %s %d\n", scmi_status_code[-ret], ret);
			fprintf(stderr, "Invalid clock arguments\n");
			help(HELP_SET_CLOCK);
		}
	} else if (!strncmp(argv[0], "parent_clock", 5)) {
		argc--;
		argv++;
		ret = set_clock_parent(argc, argv);
		if (ret) {
			if (soc_info.protocol == SCMI)
				fprintf(stderr, "SCMI_ERROR: %s %d\n", scmi_status_code[-ret], ret);
			if (ret == -1)
				fprintf(stderr, "Invalid parent_clock arguments\n");
			help(HELP_SET_CLOCK_PARENT);
		}
	} else if (!strcmp(argv[0], "--help")) {
		help(HELP_SET);
		return 0;
	} else {
		fprintf(stderr, "Invalid argument %s\n", argv[1]);
		help(HELP_SET);
		return -1;
	}
	return ret;
}
