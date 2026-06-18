/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * DMA-BUF contiguous buffer physical address user-space exporter
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 */

#ifndef DMA_BUF_PHYS_H
#define DMA_BUF_PHYS_H

#include <linux/ioctl.h>
#include <linux/types.h>

/**
 * struct dma_buf_phys_data - metadata passed from userspace for conversion
 * @fd:		DMA-BUF fd for conversion
 * @phys:	populated with CPU physical address of DMA-BUF
 */
struct dma_buf_phys_data {
	__u32 fd;
	__u64 phys;
};

#define RPROC_MAGIC	0xB7

/**
 * DOC: RPROC_IOC_DMA_BUF_ATTACH - Convert DMA-BUF to physical address
 *
 * Takes a dma_buf_phys_data struct containing a fd for a physicaly contigous
 * buffer. Pins this buffer and populates phys field with the CPU physical address.
 */
#define RPROC_IOC_DMA_BUF_ATTACH _IOWR(RPROC_MAGIC, 0, struct dma_buf_phys_data)

#endif /* DMA_BUF_PHYS_H */
