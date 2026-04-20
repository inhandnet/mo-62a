// SPDX-License-Identifier: BSD-3-Clause
/*
 * K3CONF Command Enable
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

static int enable_device(int argc, char *argv[])
{
	uint32_t dev_id, ret;

	if (argc < 1)
		return -1;

	ret = sscanf(argv[0], "%u", &dev_id);
	if (ret != 1)
		return -1;

	if (soc_info.protocol == TISCI)
		ret = ti_sci_cmd_enable_device(dev_id);
	else
		ret = scmi_cmd_enable_device(dev_id);

	if (ret)
		return ret;

	return dump_devices_info(argc, argv);
}

static int enable_clock(int argc, char *argv[])
{
	uint32_t dev_id, clk_id, ret;

	if (argc < 2)
		return -1;

	ret = sscanf(argv[0], "%u", &dev_id);
	if (ret != 1)
		return -1;

	ret = sscanf(argv[1], "%u", &clk_id);
	if (ret != 1)
		return -1;

	if (soc_info.protocol == TISCI)
		ret = ti_sci_cmd_get_clk(dev_id, clk_id);
	else
		/* dev_id is ignored for SoCs using SCMI */
		ret = scmi_cmd_enable_clk(clk_id);

	if (ret)
		return ret;

	return dump_clocks_info(1, argv);
}

int process_enable_command(int argc, char *argv[])
{
	int ret;

	if (argc < 1) {
		help(HELP_ENABLE);
		return -1;
	}

	if (!strncmp(argv[0], "device", 6)) {
		argc--;
		argv++;
		ret = enable_device(argc, argv);
		if (ret) {
			if (soc_info.protocol == SCMI)
				fprintf(stderr, "SCMI_ERROR: %s %d\n", scmi_status_code[-ret], ret);
			fprintf(stderr, "Invalid device arguments\n");
			help(HELP_ENABLE_DEVICE);
		}
	} else if (!strncmp(argv[0], "clock", 5)) {
		argc--;
		argv++;
		ret = enable_clock(argc, argv);
		if (ret) {
			if (soc_info.protocol == SCMI)
				fprintf(stderr, "SCMI_ERROR: %s %d\n", scmi_status_code[-ret], ret);
			fprintf(stderr, "Invalid clock arguments\n");
			help(HELP_ENABLE_CLOCK);
		}
	} else if (!strcmp(argv[0], "--help")) {
		help(HELP_ENABLE);
		return 0;
	} else {
		fprintf(stderr, "Invalid argument %s\n", argv[1]);
		help(HELP_ENABLE);
		return -1;
	}
	return ret;
}
