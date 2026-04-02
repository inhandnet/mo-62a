// SPDX-License-Identifier:     GPL-2.0+
/*
 * K3: ARM64 MMU setup
 *
 * Copyright (C) 2018-2020 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *	Suman Anna <s-anna@ti.com>
 * (This file is derived from arch/arm/mach-zynqmp/cpu.c)
 *
 */

#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <mach/k3-ddr.h>

DECLARE_GLOBAL_DATA_PTR;

/* We need extra 5 entries for:
 * SoC peripherals, flash, atf-carveout, tee-carveout and the sentinel value.
 */
#define K3_MMU_REGIONS_COUNT ((CONFIG_NR_DRAM_BANKS) + 5)

struct mm_region k3_mem_map[K3_MMU_REGIONS_COUNT] = {
	{ /* SoC Peripherals */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, { /* Flash peripherals */
		.virt = 0x500000000UL,
		.phys = 0x500000000UL,
		.size = 0x380000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, { /* Map SPL load region and the next 128MiB as cacheable */
		.virt = CONFIG_SPL_TEXT_BASE,
		.phys = CONFIG_SPL_TEXT_BASE,
		.size = SZ_128M,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, { /* List terminator */
		0,
	}
};

struct mm_region *mem_map = k3_mem_map;

static void k3_mmu_add_cachable_entry(u64 start, u64 end, unsigned int *map_idx)
{
	if (start >= end)
		return;

	k3_mem_map[*map_idx].virt = start,
	k3_mem_map[*map_idx].phys = start,
	k3_mem_map[*map_idx].size = end - start,
	k3_mem_map[*map_idx].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
				     PTE_BLOCK_INNER_SHARE;
	(*map_idx)++;
}

/* It is assumed that if ATF and OPTEE are loaded in DDR, they are loaded to
 * first bank only
 */
static int k3_setup_first_mem_bank(unsigned int *map_idx)
{
	struct fdt_resource mem, atf, tee, rsvd1, rsvd2;
	void *fdt = (void *)gd->fdt_blob;
	fdt_size_t size;
	int ret = 0;

	mem.start = gd->ram_base;
	mem.end = mem.start + gd->ram_size;

	atf.start = CONFIG_K3_ATF_LOAD_ADDR;
	ret = fdt_path_offset(fdt, "/reserved-memory/tfa");
	if (ret < 0)
		/* 62l doesn't have reserved-memory in the dt */
		size = 0x200000;
	else
		fdtdec_get_addr_size(fdt, ret, "reg", &size);
	atf.end = atf.start + size;

	tee.start = CONFIG_K3_OPTEE_LOAD_ADDR;
	ret = fdt_path_offset(fdt, "/reserved-memory/optee");
	if (ret < 0)
		/* 62l doesn't have reserved-memory in the dt */
		size = 0x800000;
	else
		fdtdec_get_addr_size(fdt, ret, "reg", &size);
	tee.end = tee.start + size;

	/* set reserved region lower in ddr as rsvd1 and other as rsvd2 */
	if (atf.start < tee.start)
		rsvd1 = atf, rsvd2 = tee;
	else
		rsvd1 = tee, rsvd2 = atf;

	if (rsvd2.start < mem.start) {
		/* both reserved regions lie outside DDR */
		k3_mmu_add_cachable_entry(mem.start, mem.end, map_idx);
		return 0;
	}

	if (rsvd1.start >= mem.start) {
		k3_mmu_add_cachable_entry(mem.start, rsvd1.start, map_idx);
		k3_mmu_add_cachable_entry(rsvd1.end, rsvd2.start, map_idx);
	} else {
		k3_mmu_add_cachable_entry(mem.start, rsvd2.start, map_idx);
	}

	k3_mmu_add_cachable_entry(rsvd2.end, mem.end, map_idx);

	return 0;
}

static int k3_setup_extra_mem_banks(unsigned int *map_idx)
{
	unsigned int bank;
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;

	for (bank = 1; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		k3_mmu_add_cachable_entry(gd->bd->bi_dram[bank].start,
					  gd->bd->bi_dram[bank].start +
						  gd->bd->bi_dram[bank].size,
					  map_idx);
	}

	return 0;
}

static void k3_spl_mem_map_init(unsigned int *map_idx)
{
	if (CONFIG_IS_ENABLED(VIDEO))
		k3_mmu_add_cachable_entry(gd_video_bottom(), gd_video_top(),
					  map_idx);
}

static int k3_uboot_mem_map_init(unsigned int *map_idx)
{
	int ret;

	/* Overwrite the 128MiB SPL entry */
	(*map_idx)--;

	ret = k3_setup_first_mem_bank(map_idx);
	if (ret)
		return ret;

	if (CONFIG_NR_DRAM_BANKS > 1)
		ret = k3_setup_extra_mem_banks(map_idx);

	return ret;
}

int k3_mem_map_init(void)
{
	int ret = 0;
	unsigned int map_idx, i;

	for (i = 0; i < K3_MMU_REGIONS_COUNT; i++)
		if (k3_mem_map[i].virt == CONFIG_SPL_TEXT_BASE)
			map_idx = i;

	map_idx++;

	if (xpl_phase() == PHASE_SPL)
		k3_spl_mem_map_init(&map_idx);
	else
		ret = k3_uboot_mem_map_init(&map_idx);

	if (ret)
		return ret;

	k3_mem_map[map_idx] = (const struct mm_region){ 0 };

	debug("%s: MMU Table configured as:\n", __func__);
	debug("   |virt start\t\t|virt end\t|phys\t\t|size\t\t|attrs:\n");
	for (i = 0; i < map_idx; i++) {
		debug("%2d: 0x%-12llx\t0x%-12llx\t0x%-12llx\t0x%-12llx\t0x%llx\n",
		      i, k3_mem_map[i].virt,
		      k3_mem_map[i].virt + k3_mem_map[i].size,
		      k3_mem_map[i].phys, k3_mem_map[i].size,
		      k3_mem_map[i].attrs);
	}

	return 0;
}
