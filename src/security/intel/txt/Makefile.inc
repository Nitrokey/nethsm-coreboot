romstage-$(CONFIG_INTEL_TXT_LIB) += txtlib.c

ifeq ($(CONFIG_INTEL_TXT_COMMON),y)

all-y += logging.c

romstage-y += romstage.c
romstage-y += getsec_sclean.S
romstage-y += getsec.c

romstage-y += common.c

ramstage-y += common.c
ramstage-y += getsec.c
ramstage-y += getsec_enteraccs.S
ramstage-y += ramstage.c

ifeq ($(CONFIG_CPU_INTEL_FIRMWARE_INTERFACE_TABLE),y)

# CBnT does not use FIT for IBB
ifeq ($(CONFIG_INTEL_TXT),y)
# Initial BootBlock files
ibb-files := $(foreach file,$(cbfs-files), \
	$(if $(shell echo '$(call extract_nth,7,$(file))'|grep -- --ibb), \
		$(call extract_nth,2,$(file)),))

ibb-files += bootblock

$(call add_intermediate, add_ibb_fit, $(IFITTOOL) set_fit_ptr)
	$(foreach file, $(ibb-files), $(shell $(IFITTOOL) -f $< -a -n $(file) -t 7 \
		-s $(CONFIG_CPU_INTEL_NUM_FIT_ENTRIES) -r COREBOOT)) true

endif # INTEL_TXT

endif # CPU_INTEL_FIRMWARE_INTERFACE_TABLE

endif # INTEL_TXT
