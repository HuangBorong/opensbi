/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 * Copyright (c) 2022 Ventana Micro Systems Inc.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <libfdt.h>
#include <sbi/riscv_asm.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_heap.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/irqchip/fdt_irqchip.h>
#include <sbi_utils/irqchip/aplic.h>
#include <sbi/sbi_console.h>

static void dump_aplic_data(struct aplic_data *aplic)
{
	sbi_printf("APLIC %s-mode interrupt domain at: 0x%lx, size: 0x%lx\n", 
		aplic->targets_mmode ? "M" : "S", aplic->addr, aplic->size);
	sbi_printf("num IRQ sources: %lu\n" , aplic->num_source);
	sbi_printf("num IDCs: %lu\n" , aplic->num_idc);
	sbi_printf("MMSIADDRCFG: %s\n", aplic->has_msicfg_mmode ? "true" : "false");
	sbi_printf("SMSIADDRCFG: %s\n", aplic->has_msicfg_smode ? "true" : "false");

	for (int i = 0; i < APLIC_MAX_DELEGATE; i++)
	{
		if (aplic->delegate[i].first_irq || aplic->delegate[i].last_irq || aplic->delegate[i].child_index)
		{
			sbi_printf("delegate[%d]: child_index: %u, first_irq: %u, last_irq: %u\n", 
				i, aplic->delegate[i].child_index, aplic->delegate[i].first_irq, aplic->delegate[i].last_irq);
		}		
	}

	sbi_printf("\n");
}

static int irqchip_aplic_cold_init(const void *fdt, int nodeoff,
				   const struct fdt_match *match)
{
	int rc;
	struct aplic_data *pd;

	pd = sbi_zalloc(sizeof(*pd));
	if (!pd)
		return SBI_ENOMEM;

	rc = fdt_parse_aplic_node(fdt, nodeoff, pd);
	if (rc)
		goto fail_free_data;

	dump_aplic_data(pd);

	rc = aplic_cold_irqchip_init(pd);
	if (rc)
		goto fail_free_data;

	return 0;

fail_free_data:
	sbi_free(pd);
	return rc;
}

static const struct fdt_match irqchip_aplic_match[] = {
	{ .compatible = "riscv,aplic" },
	{ },
};

struct fdt_irqchip fdt_irqchip_aplic = {
	.match_table = irqchip_aplic_match,
	.cold_init = irqchip_aplic_cold_init,
};
