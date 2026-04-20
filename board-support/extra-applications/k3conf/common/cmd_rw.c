// SPDX-License-Identifier: BSD-3-Clause
/*
 * K3CONF Command Read and write
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <tisci.h>
#include <socinfo.h>
#include <help.h>
#include <k3conf.h>
#include <mmio.h>
#include <inttypes.h>

int process_read_command(int argc, char *argv[])
{
	uint64_t addr = 0;
	int ret, num_read_bits = 32;
	uint64_t val;

	if (argc < 1) {
		help(HELP_READ);
		return -1;
	}

	ret = sscanf(argv[0], "%" SCNx64, &addr);
	if (ret != 1) {
		help(HELP_READ);
		return -1;
	}

	if (argc == 2) {
		ret = sscanf(argv[1], "%d", &num_read_bits);
		if (ret != 1) {
			help(HELP_READ);
			return -1;
		}
	}

	switch (num_read_bits) {
	case 8:
		val = mmio_read_8(addr);
		break;
	case 16:
		val = mmio_read_16(addr);
		break;
	case 32:
		val = mmio_read_32(addr);
		break;
	case 64:
		val = mmio_read_64(addr);
		break;
	default:
		fprintf(stdout, "Wrong input size, expected input size is 8,16,32,64\n");
		return -1;
	};

	fprintf(stdout, "Value at addr 0x%" PRIx64 " = 0x%" PRIx64 "\n\n", addr,
		val);

	return 0;
}

int process_write_command(int argc, char *argv[])
{
	uint64_t val;
	uint64_t addr;
	int ret, num_write_bits = 32;

	if (argc < 2) {
		help(HELP_WRITE);
		return -1;
	}

	ret = sscanf(argv[0], "%" SCNx64, &addr);
	if (ret != 1) {
		help(HELP_WRITE);
		return -1;
	}

	ret = sscanf(argv[1], "%" SCNx64, &val);
	if (ret != 1) {
		help(HELP_WRITE);
		return -1;
	}

	if (argc == 3) {
		ret = sscanf(argv[2], "%d", &num_write_bits);
		if (ret != 1) {
			help(HELP_READ);
			return -1;
		}
	}

	switch (num_write_bits) {
	case 8:
		mmio_write_8(addr, val);
		val = mmio_read_8(addr);
		break;
	case 16:
		mmio_write_16(addr, val);
		val = mmio_read_16(addr);
		break;
	case 32:
		mmio_write_32(addr, val);
		val = mmio_read_32(addr);
		break;
	case 64:
		mmio_write_64(addr, val);
		val = mmio_read_64(addr);
		break;
	default:
		fprintf(stdout, "Wrong input size, expected input size is 8,16,32,64\n");
		return -1;
	};

	fprintf(stdout, "Value at addr 0x%" PRIx64 " = 0x%" PRIx64 "\n\n", addr,
		val);

	return 0;
}
