// SPDX-License-Identifier: BSD-3-Clause
/*
 * Help Library for K3CONF
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <help.h>
#include <stdio.h>

#define HELP_CLK_SET_PARENT_URL1 "http://downloads.ti.com/tisci/esd/latest/2_tisci_msgs/pm/clocks.html#power-management-clock-frequency-configuration-example-with-mux-programming"
#define HELP_CLK_SET_PARENT_URL2 "http://downloads.ti.com/tisci/esd/latest/2_tisci_msgs/pm/clocks.html#tisci-msg-set-clock-parent"
#define HELP_DEV_ID_IGNORED "\t\tNOTICE: On SoCs using SCMI, set <dev_id> value to 0. All other parameters should be set as desired.\n"
#define HELP_PROC_ID_IGNORED "\t\tNOTICE: On SoCs using SCMI, set <proc_id> value to 0. All other parameters should be set as desired.\n"

void help(help_category cat)
{
	if (cat >= HELP_CATEGORY_MAX) {
		fprintf(stderr, "help called with incorrect category!!! (%d)\n",
			cat);
		return;
	}

	if ((cat == HELP_ALL) || (cat == HELP_USAGE)) {
		printf("\n");
		printf("NAME\n");
		printf("	k3conf - TI K3 Configuration Diagnostic Tool\n");
		printf("\n");
		printf("SYNOPSIS\n");
		printf("	k3conf [--host <id>] [--version] [--help] [--cpuinfo] <command> [<args>]\n");
		if (cat == HELP_USAGE) {
			printf("\n");
			printf("	See 'k3conf --help' for more information.\n");
			printf("\n");
		}
	}

	if (cat == HELP_ALL) {
		printf("\n");
		printf("DESCRIPTION\n");
		printf("	k3conf is standalone application designed to provide a quick'n easy way to \n"
			"	diagnose/debug/audit TI K3 architecture based processors configuration at\n"
			"	runtime, with no particular kernel dependency.\n");
		printf("	k3conf is designed to be as much platform-agnostic as possible, being able to \n"
			"	run on any Linux platform and easily ported to other OS.\n");
		printf("	Even if k3conf today focuses mainly on TISCI related functionality, it is \n"
			"	intended to be extended to any other area.\n");

		printf("\n");
		printf("OPTIONS\n");
		printf("	--host <id>\n");
		printf("	    Use this host id for communicating with sysfw\n");
		printf("\n");
		printf("	--help\n");
		printf("	    Print k3conf help.\n");
		printf("\n");
		printf("	--version\n");
		printf("	    Print k3conf version.\n");
		printf("\n");
		printf("	--cpuinfo\n");
		printf("	    Print the host processor information.\n");
	}

	if (cat != HELP_USAGE) {
		printf("\n");
		printf("COMMANDS\n");
	}

	if ((cat == HELP_ALL) || (cat == HELP_SHOW) ||
	    (cat == HELP_SHOW_HOST)) {
		printf("\n");
		printf("	k3conf show hosts\n");
		printf("		Prints all the available TISCI hosts\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_SHOW) ||
	    (cat == HELP_SHOW_SEC_PROXY)) {
		printf("\n");
		printf("	k3conf show threads\n");
		printf("		Prints all the available TISCI secure proxy threads\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_SHOW) ||
	    (cat == HELP_SHOW_DEVICE)) {
		printf("\n");
		printf("	k3conf show device\n");
		printf("		Prints all the available devices\n");
		printf("\n");
		printf("	k3conf show device <dev_id>\n");
		printf("		Prints the corresponding device id information\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_SHOW) ||
	    (cat == HELP_SHOW_CLOCK)) {
		printf("\n	k3conf show clock\n");
		printf("		Prints all the available clocks\n");
		printf("\n	k3conf show clock <dev_id>\n");
		printf("		Prints the clocks for corresponding device id\n");
		printf(HELP_DEV_ID_IGNORED);
		printf("\n	k3conf show clock <dev_id> <clk_id>\n");
		printf("		Prints the clock for corresponding device id and clock id\n");
		printf(HELP_DEV_ID_IGNORED);
	}
	if ((cat == HELP_ALL) || (cat == HELP_SHOW) ||
	    (cat == HELP_SHOW_PROCESSOR)) {
		printf("\n");
		printf("	k3conf show processor\n");
		printf("		Prints all the available processors\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_SHOW) ||
	    (cat == HELP_SHOW_RM)) {
		printf("\n");
		printf("	k3conf show rm\n");
		printf("		Prints resources managed by System firmware\n");
		printf("\n");
		printf("	k3conf show rm <dev_id>\n");
		printf("		Prints resources managed by System firmware for corresponding device\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_SHOW) ||
	    (cat == HELP_SHOW_MSMC)) {
		printf("\n");
		printf("	k3conf show msmc\n");
		printf("		Prints the valid MSMC memory area if run on applicable SoC\n");
		printf("		Shows this help message if MSMC is not valid for the SoC\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_DUMP) ||
	    (cat == HELP_DUMP_DEVICE)) {
		printf("\n");
		printf("	k3conf dump device\n");
		printf("		Prints device status of all the devices\n");
		printf("\n");
		printf("	k3conf dump device <dev_id>\n");
		printf("		Prints the corresponding device id status\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_DUMP) ||
	    (cat == HELP_DUMP_CLOCK)) {
		printf("\n");
		printf("	k3conf dump clock\n");
		printf("		Prints clock status of all the available clocks\n");
		printf("\n");
		printf("	k3conf dump clock <dev_id>\n");
		printf("		Prints the available clock status for corresponding device id\n");
		printf(HELP_DEV_ID_IGNORED);
		printf("\n");
		printf("	k3conf dump clock <dev_id> <clk_id>\n");
		printf("		Prints the available clock status for corresponding device id and clock id\n");
		printf(HELP_DEV_ID_IGNORED);
	}
	if ((cat == HELP_ALL) || (cat == HELP_DUMP) ||
	    (cat == HELP_DUMP_CLOCK_PARENT)) {
		printf("\n");
		printf("	k3conf dump parent_clock <dev_id> <clk_id>\n");
		printf("		Prints the clock parent of provided clock\n");
		printf(HELP_DEV_ID_IGNORED);
	}
	if ((cat == HELP_ALL) || (cat == HELP_DUMP) ||
	    (cat == HELP_DUMP_PROCESSOR)) {
		printf("\n");
		printf("	k3conf dump processor\n");
		printf("		Prints status of all the available processors\n");
		printf("\n");
		printf("	k3conf dump processor <proc_id>\n");
		printf("		Prints status of the given processor\n");
		printf(HELP_PROC_ID_IGNORED);
	}
	if ((cat == HELP_ALL) || (cat == HELP_DUMP) ||
	    (cat == HELP_DUMP_RM)) {
		printf("\n");
		printf("	k3conf dump rm [OPTIONS]\n");
		printf("		Prints resource allocation for all utypes / hosts\n");
		printf("\n");
		printf("	k3conf dump rm [OPTIONS] <type>\n");
		printf("		Prints resource allocation for corresponding type\n");
		printf("\n");
		printf("	k3conf dump rm [OPTIONS] <type> <subtype>\n");
		printf("		Prints resource allocation for corresponding device/type\n");
		printf("\n");
		printf("		[OPTIONS]\n");
		printf("		-h <host_id>\n");
		printf("			Filter only for corresponding host_id\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_ENABLE) ||
	    (cat == HELP_ENABLE_DEVICE)) {
		printf("\n");
		printf("	k3conf enable device <dev_id>\n");
		printf("		Enables the device and prints the status\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_ENABLE) ||
	    (cat == HELP_ENABLE_CLOCK)) {
		printf("\n");
		printf("	k3conf enable clock <dev_id> <clk_id>\n");
		printf("		Enables the clock and prints the status\n");
		printf(HELP_DEV_ID_IGNORED);
	}
	if ((cat == HELP_ALL) || (cat == HELP_DISABLE) ||
	    (cat == HELP_DISABLE_DEVICE)) {
		printf("\n");
		printf("	k3conf disable device <dev_id>\n");
		printf("		Disables the device and prints the status\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_DISABLE) ||
	    (cat == HELP_DISABLE_CLOCK)) {
		printf("\n");
		printf("	k3conf disable clock <dev_id> <clk_id>\n");
		printf("		Disables the clock and prints the status\n");
		printf(HELP_DEV_ID_IGNORED);
	}
	if ((cat == HELP_ALL) || (cat == HELP_SET) ||
	    (cat == HELP_SET_CLOCK)) {
		printf("\n");
		printf("	k3conf set clock <dev_id> <clk_id> <freq>\n");
		printf("		Sets the clock frequency and prints the status\n");
		printf(HELP_DEV_ID_IGNORED);
	}
	if ((cat == HELP_ALL) || (cat == HELP_SET) ||
	    (cat == HELP_SET_CLOCK_PARENT)) {
		printf("\n");
		printf("	k3conf set parent_clock <dev_id> <clk_id> <parent_clk_id>\n");
		printf("		Sets the parent clock for a clock mux and prints the mux status\n");
		printf("		Refer to the following documentation for preconditions:\n");
		printf("		%s\n", HELP_CLK_SET_PARENT_URL1);
		printf("		%s\n", HELP_CLK_SET_PARENT_URL2);
		printf(HELP_DEV_ID_IGNORED);
	}
	if ((cat == HELP_ALL) || (cat == HELP_READ)) {
		printf("\n");
		printf("	k3conf read <addr> [<size>]\n");
		printf("		No.of bits to be read is given in the size argument\n");
		printf("		Expected input size is 8,16,32,64\n");
		printf("		Prints the value at the specified io memory\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_WRITE)) {
		printf("\n");
		printf("	k3conf write <addr> <val> [<size>]\n");
		printf("		No.of bits to be written is given in the size argument\n");
		printf("		Expected input size is 8,16,32,64\n");
		printf("		Writes the value at the specified io memory\n");
	}
	if ((cat == HELP_ALL) || (cat == HELP_DUMP_DDRBW)) {
		printf("\n");
		printf("	k3conf ddrbw <sample_duration> <count>\n");
		printf("		Prints DDR bandwidth utilization on all DDR instances\n");
		printf("		sample_duration is delay between samples in seconds (default: 1 second)\n");
		printf("		count is number of samples to capture before stopping (default: -1 infinite)\n");
	}
}
