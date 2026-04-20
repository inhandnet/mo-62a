/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * MMIO helper library header file.
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *	Nishanth Menon <nm@ti.com>
 */

#ifndef __MMIO_H
#define __MMIO_H

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>

void mmio_write_8(uintptr_t addr, uint8_t value);

uint8_t mmio_read_8(uintptr_t addr);

void mmio_write_16(uintptr_t addr, uint16_t value);

uint16_t mmio_read_16(uintptr_t addr);

void mmio_write_32(uintptr_t addr, uint32_t value);

uint32_t mmio_read_32(uintptr_t addr);

void mmio_write_64(uintptr_t addr, uint64_t value);

uint64_t mmio_read_64(uintptr_t addr);

static inline void mmio_clrbits_32(uintptr_t addr, uint32_t clear)
{
	mmio_write_32(addr, mmio_read_32(addr) & ~clear);
}

static inline void mmio_setbits_32(uintptr_t addr, uint32_t set)
{
	mmio_write_32(addr, mmio_read_32(addr) | set);
}

static inline void mmio_clrsetbits_32(uintptr_t addr,
				      uint32_t clear, uint32_t set)
{
	mmio_write_32(addr, (mmio_read_32(addr) & ~clear) | set);
}

#define NON_ROOT_USER (-13)

#endif /* __MMIO_H */
