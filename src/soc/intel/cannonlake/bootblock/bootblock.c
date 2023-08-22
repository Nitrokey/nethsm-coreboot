/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <gpio.h>
#include <intelblocks/fast_spi.h>
#include <intelblocks/gspi.h>
#include <intelblocks/systemagent.h>
#include <intelblocks/tco.h>
#include <intelblocks/uart.h>
#include <soc/bootblock.h>
#include <soc/iomap.h>
#include <soc/pch.h>
#include <security/intel/cbnt/cbnt.h>

asmlinkage void bootblock_c_entry(uint64_t base_timestamp)
{
	/* Call lib/bootblock.c main */
	bootblock_main_with_basetime(base_timestamp);
}

void bootblock_soc_early_init(void)
{
	bootblock_systemagent_early_init();
	bootblock_pch_early_init();
	fast_spi_cache_bios_region();
	pch_early_iorange_init();
	if (CONFIG(INTEL_LPSS_UART_FOR_CONSOLE))
		uart_bootblock_init();
}

void bootblock_soc_init(void)
{
	report_platform_info();
	bootblock_pch_init();

	/* Program TCO_BASE_ADDRESS and TCO Timer Halt */
	tco_configure();

	if (CONFIG(INTEL_CBNT_LOGGING))
		intel_cbnt_log_registers();
}
