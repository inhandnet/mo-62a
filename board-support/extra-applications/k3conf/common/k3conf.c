// SPDX-License-Identifier: BSD-3-Clause
/*
 * K3CONF main entry file
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sec_proxy.h>
#include <tisci.h>
#include <version.h>
#include <socinfo.h>
#include <help.h>
#include <string.h>
#include <autoadjust_table.h>
#include <k3conf.h>

#ifdef DEBUG
#define dprintf(format, ...)	 printf(format, ## __VA_ARGS__)
#else
#define dprintf(format, ...)
#endif

#define MAX_CAPS_DECODE 9
static const char *caps_array[MAX_CAPS_DECODE] = {
	"GEN",
	"DEEP_SLP",
	"MCU_ONLY",
	"STDBY",
	"PART_IO",
	"DM_MGD_LPM",
	"IO+DDR_RET",
	"IO_ISO",
	"DM-SPLT"
};

void k3conf_print_version(FILE *stream)
{
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN];
	struct ti_sci_version_info tisci_ver = soc_info.sci_info.version;
	struct arm_scmi_version_info scmi_ver = soc_info.scmi_info.version;
	uint32_t row = 0;

	if (stream == NULL) {
		fprintf(stderr, "%s(): stream == NULL!!!\n", __func__);
		return;
	}

	autoadjust_table_init(table);
	strncpy(table[row][0], "VERSION INFO", TABLE_MAX_ELT_LEN);
	row++;
	strncpy(table[row][0], "K3CONF", TABLE_MAX_ELT_LEN);
	snprintf(table[row][1], TABLE_MAX_ELT_LEN, "(version %s built %s)", k3conf_version, builddate);
	row++;

	/* Following information is only available if soc_info is valid */
	if (!soc_info_valid)
		goto no_chip_info;

	strncpy(table[row][0], "SoC", TABLE_MAX_ELT_LEN);
	snprintf(table[row][1], TABLE_MAX_ELT_LEN, "%s%s", soc_info.soc_name, soc_info.rev_name);
	row++;

	if (strnlen(soc_info.dev_part_identifier, TABLE_MAX_ELT_LEN)) {
		strncpy(table[row][0], "SoC identifiers", TABLE_MAX_ELT_LEN);
		snprintf(table[row][1], TABLE_MAX_ELT_LEN, "%s", soc_info.dev_part_identifier);
		row++;
	}

	if (strnlen(soc_info.die_id, TABLE_MAX_ELT_LEN)) {
		strncpy(table[row][0], "DIE-ID", TABLE_MAX_ELT_LEN);
		snprintf(table[row][1], TABLE_MAX_ELT_LEN, "%s", soc_info.die_id);
		row++;
	}

	if (soc_info.ti_sci_enabled) {
		strncpy(table[row][0], "SYSFW", TABLE_MAX_ELT_LEN);
		snprintf(table[row][1], TABLE_MAX_ELT_LEN,
			 "ABI: %d.%d (firmware version 0x%04x '%.*s)')",
			 tisci_ver.abi_major, tisci_ver.abi_minor, tisci_ver.firmware_version,
			 (int)sizeof(tisci_ver.firmware_description),
			 tisci_ver.firmware_description);
		row++;

		if (tisci_ver.dm_info.valid) {
			strncpy(table[row][0], "DM ABI Info", TABLE_MAX_ELT_LEN);
			snprintf(table[row][1], TABLE_MAX_ELT_LEN,
				 "%d.%d", tisci_ver.dm_info.abi_major, tisci_ver.dm_info.abi_minor);
			row++;

			strncpy(table[row][0], "DM F/w rev", TABLE_MAX_ELT_LEN);
			snprintf(table[row][1], TABLE_MAX_ELT_LEN,
				 "%d.%d.%d",
				 tisci_ver.dm_info.dm_version,
				 tisci_ver.dm_info.sub_version,
				 tisci_ver.dm_info.patch_version);
			row++;

			strncpy(table[row][0], "DM Component rev", TABLE_MAX_ELT_LEN);
			snprintf(table[row][1], TABLE_MAX_ELT_LEN,
				 "RM/PM HAL:'%.*s' SCI_SERV:'%.*s'",
				 (int)sizeof(tisci_ver.dm_info.rm_pm_hal_version),
				 tisci_ver.dm_info.rm_pm_hal_version,
				 (int)sizeof(tisci_ver.dm_info.sci_server_version),
				 tisci_ver.dm_info.sci_server_version);
			row++;
		}

		if (tisci_ver.caps_info.valid && tisci_ver.caps_info.fw_caps) {
			int i;
			uint64_t fw_caps = tisci_ver.caps_info.fw_caps;
			char caps_str[TABLE_MAX_ELT_LEN];
			strncpy(table[row][0], "F/w Capabilities", TABLE_MAX_ELT_LEN);
			snprintf(caps_str, TABLE_MAX_ELT_LEN, "%#lx:", fw_caps);
			for (i = 0; i < MAX_CAPS_DECODE  && fw_caps; i++ ) {
				if (fw_caps & 0x1) {
					char this_cap[TABLE_MAX_ELT_LEN - 1];
					snprintf(this_cap, TABLE_MAX_ELT_LEN - 1, " %s", caps_array[i]);
					strncat(caps_str, this_cap, TABLE_MAX_ELT_LEN - 1);
				}
				fw_caps = fw_caps >> 1;
			}
			strncpy(table[row][1], caps_str, TABLE_MAX_ELT_LEN);
			row++;
		}
	}

	if (soc_info.scmi_enabled) {
		strncpy(table[row][0], "SCMI Implementation Version", TABLE_MAX_ELT_LEN);
		snprintf(table[row][1], TABLE_MAX_ELT_LEN, "0x%x", scmi_ver.impl_version);
		row++;
	}

no_chip_info:
	autoadjust_table_generic_fprint(stream, table, row, 2, TABLE_HAS_TITLE);

	return;
}

int main(int argc, char *argv[])
{
	uint32_t host_id;
	int ret = 0;

	/* Scan user arguments for options */
	argc--;
	argv++;

	if (argc == 0) {
		help(HELP_USAGE);
		ret = -1;
		goto main_exit;
	}

	if (!strcmp(argv[0], "--help")) {
		help(HELP_ALL);
		goto main_exit;
	}

	host_id = INVALID_HOST_ID;
	if (!strcmp(argv[0], "--host")) {
		argc--; argv++;
		ret = sscanf(argv[0], "%u", &host_id);
		if (ret != 1) {
			fprintf(stderr, "Invalid host id %s\n", argv[0]);
			return -1;
		}
		dprintf("%s: host_id from user = %d\n", __func__,
			soc_info.host_id);
		argc--; argv++;
	}

	ret = soc_init(host_id);

	if (!strcmp(soc_info.soc_name, "AM62Lx")) {
		fprintf(stdout,
		"\n"
		"****************************************************************************\n"
		"****************************************************************************\n"
		"*                                                                          *\n"
		"*                           EXPERIMENTAL SUPPORT!                          *\n"
		"*                                                                          *\n"
		"****************************************************************************\n"
		"****************************************************************************\n"
		"\n");
	}

	if (!strcmp(argv[0], "--version")) {
		k3conf_print_version(stdout);
		goto main_exit;
	}

	if (ret && ret != SOC_INFO_UNKNOWN_SILICON) {
		fprintf(stderr, "Unable to execute '%s' command: Potentially no access to memory\n", argv[0]);
		goto main_exit;
	}

	if (!strcmp(argv[0], "read")) {
		argc--;
		argv++;
		k3conf_print_version(stdout);
		return process_read_command(argc, argv);
	}

	if (!strcmp(argv[0], "write")) {
		argc--;
		argv++;
		k3conf_print_version(stdout);
		return process_write_command(argc, argv);
	}

	if (ret || !soc_info_valid) {
		ret = ret ? ret :  -1;
		fprintf(stderr, "Unable to execute '%s' command - no access to memory or unable to detect silicon\n", argv[0]);
		goto main_exit;
	}

	if (!strcmp(argv[0], "--cpuinfo")) {
		k3conf_print_version(stdout);
		dump_cpu_info();
		goto main_exit;
	}

	if (!strcmp(argv[0], "show")) {
		argc--;
		argv++;
		k3conf_print_version(stdout);
		return process_show_command(argc, argv);
	}

	if (!strcmp(argv[0], "dump")) {
		argc--;
		argv++;
		k3conf_print_version(stdout);
		return process_dump_command(argc, argv);
	}

	if (!strcmp(argv[0], "enable")) {
		argc--;
		argv++;
		k3conf_print_version(stdout);
		return process_enable_command(argc, argv);
	}

	if (!strcmp(argv[0], "disable")) {
		argc--;
		argv++;
		k3conf_print_version(stdout);
		return process_disable_command(argc, argv);
	}

	if (!strcmp(argv[0], "set")) {
		argc--;
		argv++;
		k3conf_print_version(stdout);
		return process_set_command(argc, argv);
	}

	if (!strcmp(argv[0], "ddrbw")) {
		argc--;
		argv++;
		k3conf_print_version(stdout);
		return ddrbw_info(argc, argv);
	}

	fprintf(stderr, "Invalid argument %s", argv[0]);
	help(HELP_USAGE);
	ret = -1;
	/* Fallthrough */

main_exit:
	return ret;
}
