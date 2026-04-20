// SPDX-License-Identifier: BSD-3-Clause
/*
 * K3 SoC detection and helper apis
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <socinfo.h>
#include <mmio.h>
#include <string.h>
#include <sec_proxy.h>
#include <scmi_raw_interface.h>
#include <soc/am65x/am65x_host_info.h>
#include <soc/am65x/am65x_sec_proxy_info.h>
#include <soc/am65x/am65x_processors_info.h>
#include <soc/am65x/am65x_devices_info.h>
#include <soc/am65x/am65x_clocks_info.h>
#include <soc/am65x/am65x_rm_info.h>
#include <soc/am65x_sr2/am65x_sr2_host_info.h>
#include <soc/am65x_sr2/am65x_sr2_sec_proxy_info.h>
#include <soc/am65x_sr2/am65x_sr2_processors_info.h>
#include <soc/am65x_sr2/am65x_sr2_devices_info.h>
#include <soc/am65x_sr2/am65x_sr2_clocks_info.h>
#include <soc/am65x_sr2/am65x_sr2_rm_info.h>
#include <soc/j721e/j721e_host_info.h>
#include <soc/j721e/j721e_sec_proxy_info.h>
#include <soc/j721e/j721e_processors_info.h>
#include <soc/j721e/j721e_devices_info.h>
#include <soc/j721e/j721e_clocks_info.h>
#include <soc/j721e/j721e_rm_info.h>
#include <soc/j721e/j721e_ddr_info.h>
#include <soc/j7200/j7200_host_info.h>
#include <soc/j7200/j7200_sec_proxy_info.h>
#include <soc/j7200/j7200_processors_info.h>
#include <soc/j7200/j7200_devices_info.h>
#include <soc/j7200/j7200_clocks_info.h>
#include <soc/j7200/j7200_rm_info.h>
#include <soc/j7200/j7200_ddr_info.h>
#include <soc/am64x/am64x_host_info.h>
#include <soc/am64x/am64x_sec_proxy_info.h>
#include <soc/am64x/am64x_processors_info.h>
#include <soc/am64x/am64x_devices_info.h>
#include <soc/am64x/am64x_clocks_info.h>
#include <soc/am64x/am64x_rm_info.h>
#include <soc/am64x/am64x_ddr_info.h>
#include <soc/am62x/am62x_devices_info.h>
#include <soc/am62x/am62x_clocks_info.h>
#include <soc/am62x/am62x_host_info.h>
#include <soc/am62x/am62x_processors_info.h>
#include <soc/am62x/am62x_rm_info.h>
#include <soc/am62x/am62x_sec_proxy_info.h>
#include <soc/am62x/am62x_ddr_info.h>
#include <soc/j721s2/j721s2_devices_info.h>
#include <soc/j721s2/j721s2_clocks_info.h>
#include <soc/j721s2/j721s2_host_info.h>
#include <soc/j721s2/j721s2_processors_info.h>
#include <soc/j721s2/j721s2_rm_info.h>
#include <soc/j721s2/j721s2_sec_proxy_info.h>
#include <soc/j721s2/j721s2_ddr_info.h>
#include <soc/j784s4/j784s4_clocks_info.h>
#include <soc/j784s4/j784s4_devices_info.h>
#include <soc/j784s4/j784s4_host_info.h>
#include <soc/j784s4/j784s4_processors_info.h>
#include <soc/j784s4/j784s4_rm_info.h>
#include <soc/j784s4/j784s4_sec_proxy_info.h>
#include <soc/j784s4/j784s4_ddr_info.h>
#include <soc/am62ax/am62ax_clocks_info.h>
#include <soc/am62ax/am62ax_devices_info.h>
#include <soc/am62ax/am62ax_host_info.h>
#include <soc/am62ax/am62ax_processors_info.h>
#include <soc/am62ax/am62ax_rm_info.h>
#include <soc/am62ax/am62ax_sec_proxy_info.h>
#include <soc/am62ax/am62ax_ddr_info.h>
#include <soc/am62px/am62px_clocks_info.h>
#include <soc/am62px/am62px_devices_info.h>
#include <soc/am62px/am62px_host_info.h>
#include <soc/am62px/am62px_processors_info.h>
#include <soc/am62px/am62px_rm_info.h>
#include <soc/am62px/am62px_sec_proxy_info.h>
#include <soc/am62px/am62px_ddr_info.h>
#include <soc/j722s/j722s_clocks_info.h>
#include <soc/j722s/j722s_devices_info.h>
#include <soc/j722s/j722s_host_info.h>
#include <soc/j722s/j722s_processors_info.h>
#include <soc/j722s/j722s_rm_info.h>
#include <soc/j722s/j722s_sec_proxy_info.h>
#include <soc/am62lx/am62lx_clocks_info.h>
#include <soc/am62lx/am62lx_devices_info.h>
#include <soc/am62lx/am62lx_ddr_info.h>

/* Assuming these addresses and definitions stay common across K3 devices */
#define CTRLMMR_WKUP_JTAG_DEVICE_ID	0x43000018
#define DEVICE_ID_FAMILY_SHIFT	26
#define DEVICE_ID_FAMILY_MASK	(0x3f << 26)
#define DEVICE_ID_BASE_SHIFT	11
#define DEVICE_ID_BASE_MASK	(0x1fff << 11)
#define DEVICE_ID_SPEED_SHIFT	6
#define DEVICE_ID_SPEED_MASK	(0x1f << 6)
#define DEVICE_ID_TEMP_SHIFT	3
#define DEVICE_ID_TEMP_MASK	(0x7 << 3)
#define DEVICE_ID_PKG_SHIFT	0
#define DEVICE_ID_PKG_MASK	    0x7

#define CTRLMMR_WKUP_JTAG_ID		0x43000014
#define JTAG_ID_VARIANT_SHIFT	28
#define JTAG_ID_VARIANT_MASK	(0xf << 28)
#define JTAG_ID_PARTNO_SHIFT	12
#define JTAG_ID_PARTNO_MASK	(0xffff << 12)

#define CTRLMMR_WKUP_GP_SW1	0x43000234
#define GP_SW1_VARIANT_MOD_OPR	16

#define CTRLMMR_WKUP_DIE_ID0	0x43000020
#define CTRLMMR_WKUP_DIE_ID1	0x43000024
#define CTRLMMR_WKUP_DIE_ID2	0x43000028
#define CTRLMMR_WKUP_DIE_ID3	0x4300002c
#define CTRLMMR_WKUP_DEVSTAT	0x43000030
#define CTRLMMR_WKUP_BOOTCFG	0x43000034

struct k3conf_soc_info soc_info;
int soc_info_valid = 0;

typedef enum {
	REV_1,
	REV_2,
	REV_3,
	REV_PG_MAX
} k3_soc_rev;

static const char *soc_revision_j721e[] = {
	[REV_1] = " SR1.0",
	[REV_2] = " SR1.1",
	[REV_3] = " SR2.0",
};

static const char *soc_gpsw_revision_am62p[] = {
	[REV_1] = " SR1.0",
	[REV_2] = " SR1.1",
	[REV_3] = " SR1.2",
};

static const char *soc_revision_generic[] = {
	[REV_1] = " SR1.0",
	[REV_2] = " SR2.0",
};

static uint32_t generic_get_jtag_device_id(void)
{
	uint32_t val;
	char device_id[100];

	val = mmio_read_32(CTRLMMR_WKUP_JTAG_DEVICE_ID);
	snprintf(device_id, 100, "[0x%08x] ", val);

	strncat(soc_info.dev_part_identifier, device_id, TABLE_MAX_ELT_LEN - 1);

	return val;
}

static void generic_decode_device_id(uint32_t jtag_device_id)
{
	uint32_t fam;
	uint32_t base;
	char device_id[100];

	/* Device ID */
	fam = (jtag_device_id & DEVICE_ID_FAMILY_MASK) >> DEVICE_ID_FAMILY_SHIFT;
	base = (jtag_device_id & DEVICE_ID_BASE_MASK) >> DEVICE_ID_BASE_SHIFT;
	snprintf(device_id, 100, "fam: 0x%08x base: 0x%08x ", fam, base);

	strncat(soc_info.dev_part_identifier, device_id, TABLE_MAX_ELT_LEN - 1);
}

static void generic_decode_device_id_v2(uint32_t jtag_device_id)
{
	uint32_t did;
	char device_id[100];

	/* Device ID */
	did = (jtag_device_id & 0xFFFFE000) >> 13;
	snprintf(device_id, 100, "0x%x ", did);

	strncat(soc_info.dev_part_identifier, device_id, TABLE_MAX_ELT_LEN - 1);
}

static void generic_decode_safety(uint32_t jtag_device_id)
{
	uint32_t val;
	char *safe;

	val = (jtag_device_id & 0x1000) >> 12;
	if (val)
		safe = "Func-Safe ";
	else
		safe = "Non-Safe ";

	strncat(soc_info.dev_part_identifier, safe, TABLE_MAX_ELT_LEN - 1);
}

static void generic_decode_secure(uint32_t jtag_device_id)
{
	uint32_t val;
	char *secure;

	val = (jtag_device_id & 0x800) >> 11;
	if (val)
		secure = "Non-Secure ";
	else
		secure = "Secure ";

	strncat(soc_info.dev_part_identifier, secure, TABLE_MAX_ELT_LEN - 1);
}

static void generic_decode_speed(uint32_t jtag_device_id)
{
	uint32_t val;
	char speed[100];

	/* Speed designator */
	val = (jtag_device_id & DEVICE_ID_SPEED_MASK) >> DEVICE_ID_SPEED_SHIFT;
	snprintf(speed, 100, "'%c' Grade ", val - 1 + 'A');

	strncat(soc_info.dev_part_identifier, speed, TABLE_MAX_ELT_LEN - 1);
}

static void generic_decode_temp(uint32_t jtag_device_id)
{
	uint32_t val;
	char *temp;
	char tmp_str[100];

	val = (jtag_device_id & DEVICE_ID_TEMP_MASK) >> DEVICE_ID_TEMP_SHIFT;
	switch (val) {
		case 3:
			temp = "0 C to 95 C ";
			break;
		case 4:
			temp = "-40 C to 105 C ";
			break;
		case 5:
			temp = "-40 C to 125 C";
			break;
		default:
			temp = "Unknown";
			break;
	}
	snprintf(tmp_str, 100, "%s ", temp);

	strncat(soc_info.dev_part_identifier, tmp_str, TABLE_MAX_ELT_LEN - 1);
}

static uint32_t generic_decode_jtag_id(void)
{
	uint32_t jtag_id = generic_get_jtag_device_id();
	if (!jtag_id)
		return 0;
	generic_decode_device_id(jtag_id);
	generic_decode_speed(jtag_id);
	generic_decode_temp(jtag_id);

	return jtag_id;
}

static uint32_t generic_decode_jtag_id_v2(void)
{
	uint32_t jtag_id = generic_get_jtag_device_id();
	if (!jtag_id)
		return 0;
	generic_decode_device_id_v2(jtag_id);
	generic_decode_safety(jtag_id);
	generic_decode_secure(jtag_id);
	generic_decode_speed(jtag_id);
	generic_decode_temp(jtag_id);

	return jtag_id;
}

static void generic_get_die_id(void)
{
	uint32_t id0 = mmio_read_32(CTRLMMR_WKUP_DIE_ID0);
	uint32_t id1 = mmio_read_32(CTRLMMR_WKUP_DIE_ID1);
	uint32_t id2 = mmio_read_32(CTRLMMR_WKUP_DIE_ID2);
	uint32_t id3 = mmio_read_32(CTRLMMR_WKUP_DIE_ID3);

	snprintf(soc_info.die_id, TABLE_MAX_ELT_LEN - 1,
		 "[0] 0x%08X [1] 0x%08x [2] 0x%08x [3] 0x%08x",
		 id0, id1, id2, id3);
}

static void generic_tda_die_decode(void)
{
	uint32_t jtag_id;

	generic_get_die_id();

	jtag_id = generic_get_jtag_device_id();
	if (!jtag_id)
		return;
	generic_decode_device_id(jtag_id);
}

static void am654_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;

	sci_info->host_info = am65x_host_info;
	sci_info->num_hosts = AM65X_MAX_HOST_IDS;
	sci_info->sp_info[MAIN_SEC_PROXY] = am65x_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = AM65X_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = am65x_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = AM65X_MCU_SEC_PROXY_THREADS;
	sci_info->processors_info = am65x_processors_info;
	sci_info->num_processors = AM65X_MAX_PROCESSORS_IDS;
	sci_info->devices_info = am65x_devices_info;
	sci_info->num_devices = AM65X_MAX_DEVICES;
	sci_info->clocks_info = am65x_clocks_info;
	sci_info->num_clocks = AM65X_MAX_CLOCKS;
	sci_info->rm_info = am65x_rm_info;
	sci_info->num_res = AM65X_MAX_RES;
	soc_info.host_id = DEFAULT_HOST_ID;
	soc_info.sec_proxy = &k3_generic_sec_proxy_base;
	soc_info.protocol = TISCI;
}

static void am654_sr2_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;
	uint32_t jtag_id, val;
	char *pkg;
	char tmp_str[100];

	sci_info->host_info = am65x_sr2_host_info;
	sci_info->num_hosts = AM65X_SR2_MAX_HOST_IDS;
	sci_info->sp_info[MAIN_SEC_PROXY] = am65x_sr2_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = AM65X_SR2_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = am65x_sr2_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = AM65X_SR2_MCU_SEC_PROXY_THREADS;
	sci_info->processors_info = am65x_sr2_processors_info;
	sci_info->num_processors = AM65X_SR2_MAX_PROCESSORS_IDS;
	sci_info->devices_info = am65x_sr2_devices_info;
	sci_info->num_devices = AM65X_SR2_MAX_DEVICES;
	sci_info->clocks_info = am65x_sr2_clocks_info;
	sci_info->num_clocks = AM65X_SR2_MAX_CLOCKS;
	sci_info->rm_info = am65x_sr2_rm_info;
	sci_info->num_res = AM65X_SR2_MAX_RES;
	soc_info.host_id = DEFAULT_HOST_ID;
	soc_info.sec_proxy = &k3_generic_sec_proxy_base;
	soc_info.protocol = TISCI;

	jtag_id = generic_decode_jtag_id();
	val = (jtag_id & DEVICE_ID_PKG_MASK) >> DEVICE_ID_PKG_SHIFT;
	switch (val) {
		case 1:
			pkg = "ACD";
			break;
		case 7:
			pkg = "DIE only";
			break;
		default:
			pkg = "Unknown";
			break;
	}
	snprintf(tmp_str, 100, "%s Package ", pkg);

	strncat(soc_info.dev_part_identifier, tmp_str, TABLE_MAX_ELT_LEN - 1);
}

static void j721s2_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;

	sci_info->sp_info[MAIN_SEC_PROXY] = j721s2_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = J721S2_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = j721s2_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = J721S2_MCU_SEC_PROXY_THREADS;
	sci_info->rm_info = j721s2_rm_info;
	sci_info->num_res = J721S2_MAX_RES;
	sci_info->host_info = j721s2_host_info;
	sci_info->num_hosts = J721S2_MAX_HOST_IDS;
	sci_info->clocks_info = j721s2_clocks_info;
	sci_info->num_clocks = J721S2_MAX_CLOCKS;
	sci_info->devices_info = j721s2_devices_info;
	sci_info->num_devices = J721S2_MAX_DEVICES;
	sci_info->processors_info = j721s2_processors_info;
	sci_info->num_processors = J721S2_MAX_PROCESSORS_IDS;
	soc_info.host_id = DEFAULT_HOST_ID;
	soc_info.sec_proxy = &k3_generic_sec_proxy_base;
	soc_info.ddr_perf_info = &j721s2_ddr_perf_info;
	soc_info.protocol = TISCI;

	generic_tda_die_decode();
}

static void j721e_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;

	sci_info->host_info = j721e_host_info;
	sci_info->num_hosts = J721E_MAX_HOST_IDS;
	sci_info->sp_info[MAIN_SEC_PROXY] = j721e_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = J721E_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = j721e_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = J721E_MCU_SEC_PROXY_THREADS;
	sci_info->processors_info = j721e_processors_info;
	sci_info->num_processors = J721E_MAX_PROCESSORS_IDS;
	sci_info->devices_info = j721e_devices_info;
	sci_info->num_devices = J721E_MAX_DEVICES;
	sci_info->clocks_info = j721e_clocks_info;
	sci_info->num_clocks = J721E_MAX_CLOCKS;
	sci_info->rm_info = j721e_rm_info;
	sci_info->num_res = J721E_MAX_RES;
	soc_info.host_id = DEFAULT_HOST_ID;
	soc_info.sec_proxy = &k3_generic_sec_proxy_base;
	soc_info.ddr_perf_info = &j721e_ddr_perf_info;
	soc_info.protocol = TISCI;

	generic_tda_die_decode();
}

static void j7200_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;

	sci_info->host_info = j7200_host_info;
	sci_info->num_hosts = J7200_MAX_HOST_IDS;
	sci_info->sp_info[MAIN_SEC_PROXY] = j7200_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = J7200_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = j7200_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = J7200_MCU_SEC_PROXY_THREADS;
	sci_info->processors_info = j7200_processors_info;
	sci_info->num_processors = J7200_MAX_PROCESSORS_IDS;
	sci_info->devices_info = j7200_devices_info;
	sci_info->num_devices = J7200_MAX_DEVICES;
	sci_info->clocks_info = j7200_clocks_info;
	sci_info->num_clocks = J7200_MAX_CLOCKS;
	sci_info->rm_info = j7200_rm_info;
	sci_info->num_res = J7200_MAX_RES;
	soc_info.host_id = DEFAULT_HOST_ID;
	soc_info.sec_proxy = &k3_generic_sec_proxy_base;
	soc_info.ddr_perf_info = &j7200_ddr_perf_info;
	soc_info.protocol = TISCI;

	generic_tda_die_decode();
}

static void am64x_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;
	uint32_t jtag_id, val;
	char *pkg;
	char tmp_str[100];

	sci_info->host_info = am64x_host_info;
	sci_info->num_hosts = AM64X_MAX_HOST_IDS;
	sci_info->sp_info[MAIN_SEC_PROXY] = am64x_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = AM64X_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = NULL;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = 0;
	sci_info->processors_info = am64x_processors_info;
	sci_info->num_processors = AM64X_MAX_PROCESSORS_IDS;
	sci_info->devices_info = am64x_devices_info;
	sci_info->num_devices = AM64X_MAX_DEVICES;
	sci_info->clocks_info = am64x_clocks_info;
	sci_info->num_clocks = AM64X_MAX_CLOCKS;
	sci_info->rm_info = am64x_rm_info;
	sci_info->num_res = AM64X_MAX_RES;
	soc_info.host_id = 13;
	soc_info.sec_proxy = &k3_lite_sec_proxy_base;
	soc_info.ddr_perf_info = &am64x_ddr_perf_info;
	soc_info.protocol = TISCI;

	jtag_id = generic_decode_jtag_id_v2();
	val = (jtag_id & DEVICE_ID_PKG_MASK) >> DEVICE_ID_PKG_SHIFT;
	switch (val) {
		case 4:
			pkg = "ALV";
			break;
		case 5:
			pkg = "ALX";
			break;
		default:
			pkg = "Unknown";
			break;
	}
	snprintf(tmp_str, 100, "%s Package ", pkg);

	strncat(soc_info.dev_part_identifier, tmp_str, TABLE_MAX_ELT_LEN - 1);
}

static void am62x_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;
	uint32_t jtag_id, val;
	char *pkg;
	char tmp_str[100];

	sci_info->sp_info[MAIN_SEC_PROXY] = am62x_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = AM62X_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = NULL;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = 0;
	sci_info->rm_info = am62x_rm_info;
	sci_info->num_res = AM62X_MAX_RES;
	sci_info->processors_info = am62x_processors_info;
	sci_info->num_processors = AM62X_MAX_PROCESSORS_IDS;
	sci_info->host_info = am62x_host_info;
	sci_info->num_hosts = AM62X_MAX_HOST_IDS;
	sci_info->devices_info = am62x_devices_info;
	sci_info->num_devices = AM62X_MAX_DEVICES;
	sci_info->clocks_info = am62x_clocks_info;
	sci_info->num_clocks = AM62X_MAX_CLOCKS;
	soc_info.host_id = 13;
	soc_info.sec_proxy = &k3_lite_sec_proxy_base;
	soc_info.ddr_perf_info = &am62x_ddr_perf_info;
	soc_info.protocol = TISCI;

	jtag_id = generic_decode_jtag_id_v2();
	val = (jtag_id & DEVICE_ID_PKG_MASK) >> DEVICE_ID_PKG_SHIFT;
	switch (val) {
		case 1:
			pkg = "ALW";
			break;
		case 6:
			pkg = "AMC";
			break;
		default:
			pkg = "Unknown";
			break;
	}
	snprintf(tmp_str, 100, "%s Package ", pkg);

	strncat(soc_info.dev_part_identifier, tmp_str, TABLE_MAX_ELT_LEN - 1);
}

static void j722s_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;

	sci_info->clocks_info = j722s_clocks_info;
	sci_info->num_clocks = J722S_MAX_CLOCKS;
	sci_info->devices_info = j722s_devices_info;
	sci_info->num_devices = J722S_MAX_DEVICES;
	sci_info->host_info = j722s_host_info;
	sci_info->num_hosts = J722S_MAX_HOST_IDS;
	sci_info->processors_info = j722s_processors_info;
	sci_info->num_processors = J722S_MAX_PROCESSORS_IDS;
	sci_info->rm_info = j722s_rm_info;
	sci_info->num_res = J722S_MAX_RES;
	sci_info->sp_info[MAIN_SEC_PROXY] = j722s_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = J722S_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = j722s_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = J722S_MCU_SEC_PROXY_THREADS;

	soc_info.host_id = 13;
	soc_info.sec_proxy = &k3_lite_sec_proxy_base;
	soc_info.protocol = TISCI;

	generic_tda_die_decode();
}

static void j784s4_init(uint32_t pkg)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;

	sci_info->clocks_info = j784s4_clocks_info;
	sci_info->num_clocks = J784S4_MAX_CLOCKS;
	sci_info->devices_info = j784s4_devices_info;
	sci_info->num_devices = J784S4_MAX_DEVICES;
	sci_info->host_info = j784s4_host_info;
	sci_info->num_hosts = J784S4_MAX_HOST_IDS;
	sci_info->processors_info = j784s4_processors_info;
	sci_info->num_processors = J784S4_MAX_PROCESSORS_IDS;
	sci_info->rm_info = j784s4_rm_info;
	sci_info->num_res = J784S4_MAX_RES;
	sci_info->sp_info[MAIN_SEC_PROXY] = j784s4_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = J784S4_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = j784s4_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = J784S4_MCU_SEC_PROXY_THREADS;

	soc_info.host_id = DEFAULT_HOST_ID;
	soc_info.sec_proxy = &k3_generic_sec_proxy_base;
	if (pkg != DEVICE_ID_PKG_J742S2)
		soc_info.ddr_perf_info = &j784s4_ddr_perf_info;
	else
		soc_info.ddr_perf_info = &j742s2_ddr_perf_info;
	soc_info.protocol = TISCI;

	generic_tda_die_decode();
}

static void am62ax_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;
	uint32_t jtag_id, val;
	char *pkg;
	char tmp_str[100];

	sci_info->clocks_info = am62ax_clocks_info;
	sci_info->num_clocks = AM62AX_MAX_CLOCKS;
	sci_info->devices_info = am62ax_devices_info;
	sci_info->num_devices = AM62AX_MAX_DEVICES;
	sci_info->host_info = am62ax_host_info;
	sci_info->num_hosts = AM62AX_MAX_HOST_IDS;
	sci_info->processors_info = am62ax_processors_info;
	sci_info->num_processors = AM62AX_MAX_PROCESSORS_IDS;
	sci_info->rm_info = am62ax_rm_info;
	sci_info->num_res = AM62AX_MAX_RES;
	sci_info->sp_info[MAIN_SEC_PROXY] = am62ax_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = AM62AX_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = am62ax_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = AM62AX_MCU_SEC_PROXY_THREADS;

	soc_info.host_id = 13;
	soc_info.sec_proxy = &k3_lite_sec_proxy_base;
	soc_info.ddr_perf_info = &am62ax_ddr_perf_info;
	soc_info.protocol = TISCI;

	jtag_id = generic_decode_jtag_id_v2();
	val = (jtag_id & DEVICE_ID_PKG_MASK) >> DEVICE_ID_PKG_SHIFT;
	switch (val) {
		case 6:
			pkg = "AMB";
			break;
		default:
			pkg = "Unknown";
			break;
	}
	snprintf(tmp_str, 100, "%s Package ", pkg);

	strncat(soc_info.dev_part_identifier, tmp_str, TABLE_MAX_ELT_LEN - 1);
}

static void am62px_init(void)
{
	struct ti_sci_info *sci_info = &soc_info.sci_info;
	uint32_t jtag_id, val;
	char *pkg;
	char tmp_str[100];

	sci_info->clocks_info = am62px_clocks_info;
	sci_info->num_clocks = AM62PX_MAX_CLOCKS;
	sci_info->devices_info = am62px_devices_info;
	sci_info->num_devices = AM62PX_MAX_DEVICES;
	sci_info->host_info = am62px_host_info;
	sci_info->num_hosts = AM62PX_MAX_HOST_IDS;
	sci_info->processors_info = am62px_processors_info;
	sci_info->num_processors = AM62PX_MAX_PROCESSORS_IDS;
	sci_info->rm_info = am62px_rm_info;
	sci_info->num_res = AM62PX_MAX_RES;
	sci_info->sp_info[MAIN_SEC_PROXY] = am62px_main_sp_info;
	sci_info->num_sp_threads[MAIN_SEC_PROXY] = AM62PX_MAIN_SEC_PROXY_THREADS;
	sci_info->sp_info[MCU_SEC_PROXY] = am62px_mcu_sp_info;
	sci_info->num_sp_threads[MCU_SEC_PROXY] = AM62PX_MCU_SEC_PROXY_THREADS;

	soc_info.host_id = 13;
	soc_info.sec_proxy = &k3_lite_sec_proxy_base;
	soc_info.ddr_perf_info = &am62px_ddr_perf_info;
	soc_info.protocol = TISCI;

	jtag_id = generic_decode_jtag_id_v2();
	val = (jtag_id & DEVICE_ID_PKG_MASK) >> DEVICE_ID_PKG_SHIFT;
	switch (val) {
		case 6:
			pkg = "AMH";
			break;
		default:
			pkg = "Unknown";
			break;
	}
	snprintf(tmp_str, 100, "%s Package ", pkg);

	strncat(soc_info.dev_part_identifier, tmp_str, TABLE_MAX_ELT_LEN - 1);
}

static void am62lx_init(void)
{
	struct arm_scmi_info *scmi_info = &soc_info.scmi_info;
	uint32_t jtag_id, val, ddr_type;
	char *pkg;
	char tmp_str[100];

	scmi_info->clocks_info = am62lx_clocks_info;
	scmi_info->num_clocks = AM62LX_MAX_CLOCKS;
	scmi_info->devices_info = am62lx_devices_info;
	scmi_info->num_devices = AM62LX_MAX_DEVICES;
	soc_info.protocol = SCMI;

	jtag_id = generic_decode_jtag_id_v2();
	val = (jtag_id & DEVICE_ID_PKG_MASK) >> DEVICE_ID_PKG_SHIFT;

	switch (val) {
		case 5:
			pkg = "ANQ";
			break;
		case 6:
			pkg = "ANB";
			break;
		default:
			pkg = "Unknown";
			break;
	}

	snprintf(tmp_str, 100, "%s Package ", pkg);
	strncat(soc_info.dev_part_identifier, tmp_str, TABLE_MAX_ELT_LEN - 1);

	soc_info.ddr_perf_info = &am62lx_ddr_perf_info;

	/* ddr type:
	 *	0xA: ddr4, burst size is 16 bytes
	 *	0xB: lpddr4, burst size is 32 bytes
	 */
	ddr_type = mmio_read_32(AM62LX_DDR_CLASS_REG);
	ddr_type = AM62LX_DDR_CLASS_MASK(ddr_type);
	soc_info.ddr_perf_info->burst_size = (ddr_type == AM62LX_DDR_TYPE_DDR4) ? 16 : 32;
}

int soc_init(uint32_t host_id)
{
	memset(&soc_info, 0, sizeof(soc_info));

	uint32_t soc = (mmio_read_32(CTRLMMR_WKUP_JTAG_ID) &
			JTAG_ID_PARTNO_MASK) >> JTAG_ID_PARTNO_SHIFT;
	uint32_t pkg = (mmio_read_32(CTRLMMR_WKUP_JTAG_DEVICE_ID)
			& DEVICE_ID_PKG_MASK) >> DEVICE_ID_PKG_SHIFT;
	k3_soc_rev rev = (mmio_read_32(CTRLMMR_WKUP_JTAG_ID) &
			JTAG_ID_VARIANT_MASK) >> JTAG_ID_VARIANT_SHIFT;
	k3_soc_rev gpsw1_val = 0;

	switch (soc) {
	case AM65X:
		soc_info.soc_name = "AM65x";
		break;
	case J721S2:
		soc_info.soc_name = "J721S2";
		break;
	case J721E:
		soc_info.soc_name = "J721E";
		break;
	case J7200:
		soc_info.soc_name = "J7200";
		break;
	case AM64X:
		soc_info.soc_name = "AM64x";
		break;
	case AM62X:
		soc_info.soc_name = "AM62X";
		break;
	case AM62AX:
		soc_info.soc_name = "AM62Ax";
		break;
	case AM62PX:
		soc_info.soc_name = "AM62Px";
		break;
	case AM62LX:
		soc_info.soc_name = "AM62Lx";
		break;
	case J784S4:
		switch (pkg) {
			case DEVICE_ID_PKG_J784S4:
				soc_info.soc_name = "J784S4";
				break;
			case DEVICE_ID_PKG_J742S2:
				soc_info.soc_name = "J742S2";
				break;
			default:
				fprintf(stderr, "Can't detect J784s4 vs J742S2. Unknown pkg %d\n",
						pkg);
				return SOC_INFO_UNKNOWN_SILICON;
		}
		break;
	case J722S:
		soc_info.soc_name = "J722S";
		break;
	default:
		fprintf(stderr, "Unknown Silicon %d\n", soc);
		return SOC_INFO_UNKNOWN_SILICON;
	};

	if (rev >= REV_PG_MAX) {
		fprintf(stderr, "Unknown Silicon revision %d for SoC %s\n",
			rev, soc_info.soc_name);
		return SOC_INFO_UNKNOWN_SILICON;
	}

	switch (soc) {
	case J721E:
		soc_info.rev_name = soc_revision_j721e[rev];
		break;
	case AM62PX:
		gpsw1_val = mmio_read_32(CTRLMMR_WKUP_GP_SW1);
		soc_info.rev_name = soc_gpsw_revision_am62p[gpsw1_val % GP_SW1_VARIANT_MOD_OPR];
		break;
	default:
		soc_info.rev_name = soc_revision_generic[rev];
	};

	if (soc == AM65X && rev == REV_1)
		am654_init();
	else if (soc == AM65X && rev == REV_2)
		am654_sr2_init();
	else if (soc == J721S2)
		j721s2_init();
	else if (soc == J721E)
		j721e_init();
	else if (soc == J7200)
		j7200_init();
	else if (soc == AM64X)
		am64x_init();
	else if (soc == AM62X)
		am62x_init();
	else if (soc == AM62AX)
		am62ax_init();
	else if (soc == AM62PX)
		am62px_init();
	else if (soc == AM62LX)
		am62lx_init();
	else if (soc == J784S4)
		j784s4_init(pkg);
	else if (soc == J722S)
		j722s_init();

	if (host_id != INVALID_HOST_ID)
		soc_info.host_id = host_id;

	/* ToDo: Add error if sec_proxy_init/sci_init is failed */
	if (soc_info.protocol == SCMI) {
		if (!scmi_raw_init())
			if (!scmi_init())
				soc_info.scmi_enabled = 1;
	} else {
		if (!k3_sec_proxy_init())
			if (!ti_sci_init())
				soc_info.ti_sci_enabled = 1;
	}

	soc_info_valid = 1;
	return 0;
}
