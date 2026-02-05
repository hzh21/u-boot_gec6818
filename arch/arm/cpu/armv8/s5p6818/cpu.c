/*
 * (C) Copyright 2016 Nexell
 * Hyunseok, Jung <hsjung@nexell.co.kr>
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

// #include <common.h>
#include <command.h>
#include <asm/system.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <asm/io.h>
#include <asm/armv8/mmu.h>
#include <asm/arch/nexell.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef	CONFIG_ARCH_CPU_INIT
#error must be define the macro "CONFIG_ARCH_CPU_INIT"
#endif

void cpu_base_init(void)
{
	/*
	 * NOTE> ALIVE Power Gate must enable for Alive register access.
	 *	     must be clear wfi jump address
	 */
	writel(1, ALIVEPWRGATEREG);
	writel(0xFFFFFFFF, SCR_ARM_SECOND_BOOT);

	/* write 0xf0 on alive scratchpad reg for boot success check */
	writel(readl(SCR_SIGNAGURE_READ) | 0xF0, (SCR_SIGNAGURE_SET));
}

#if defined(CONFIG_ARCH_CPU_INIT)
int arch_cpu_init(void)
{
	flush_dcache_all();
	cpu_base_init();
	clk_init();
	return 0;
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	return 0;
}
#endif

void reset_cpu(void)
{
	void *clkpwr_reg = (void *)PHY_BASEADDR_CLKPWR;
	const u32 sw_rst_enb_bitpos = 3;
	const u32 sw_rst_enb_mask = 1 << sw_rst_enb_bitpos;
	const u32 sw_rst_bitpos = 12;
	const u32 sw_rst_mask = 1 << sw_rst_bitpos;
	int pwrcont = 0x224;
	int pwrmode = 0x228;
	u32 read_value;

	read_value = readl((void *)(clkpwr_reg + pwrcont));

	read_value &= ~sw_rst_enb_mask;
	read_value |= 1 << sw_rst_enb_bitpos;

	writel(read_value, (void *)(clkpwr_reg + pwrcont));
	writel(sw_rst_mask, (void *)(clkpwr_reg + pwrmode));
}

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	return 0;
}
#endif	/* CONFIG_ARCH_MISC_INIT */
/* * S5P6818 内存映射表
 * 必须在这里定义，以确保 SoC 核心层级能够识别 DREX 寄存器区
 */
static struct mm_region s5p6818_mem_map[] = {
	{
		/* 映射 2GB 内存区间 (0x40000000 ~ 0xBFFFFFFF) */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x80000000UL, 
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* 映射寄存器区 (0xC0000000 开始，包含 DREX 所在位置) */
		.virt = 0xc0000000UL,
		.phys = 0xc0000000UL,
		.size = 0x02000000UL, 
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* 结束标志 */
		0,
	}
};

/* 声明全局指针，让 U-Boot 启动时加载这张表 */
struct mm_region *mem_map = s5p6818_mem_map;
