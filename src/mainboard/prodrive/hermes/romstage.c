/* SPDX-License-Identifier: GPL-2.0-only */

#include <soc/cnl_memcfg_init.h>
#include <soc/romstage.h>
#include <variant/variants.h>
#include "variants/baseboard/include/eeprom.h"

void mainboard_memory_init_params(FSPM_UPD *memupd)
{
	memupd->FspmConfig.UserBd = BOARD_TYPE_SERVER;
	memupd->FspmTestConfig.SmbusSpdWriteDisable = 0;
	cannonlake_memcfg_init(&memupd->FspmConfig, variant_memcfg_config());

	memupd->FspmConfig.RMT = 0;
	memupd->FspmConfig.HyperThreading = 1;
	memupd->FspmConfig.BootFrequency = 2;
	memupd->FspmTestConfig.VtdDisable = 0;
}
