ifeq ($(CONFIG_INTEL_BG_SUPPORT),y)

all-y += ../cbnt/logging.c
all-y += ../txt/logging.c

# As specified in Intel Boot Guard BIOS
# Specification, document number # 558294
PK_HASH_ALG_SHA256:=11

# The private key also contains the public key, so use that if a private key is provided.
ifeq ($(CONFIG_INTEL_BG_NEED_KM_PRIV_KEY),y)
$(obj)/km_pub.pem: $(call strip_quotes, $(CONFIG_INTEL_BG_KM_PRIV_KEY_FILE))
	openssl pkey -in $< -pubout > $@
else ifeq ($(CONFIG_INTEL_BG_NEED_KM_PUB_KEY),y)
$(obj)/km_pub.pem: $(call strip_quotes, $(CONFIG_INTEL_BG_KM_PUB_KEY_FILE))
	cp $< $@
endif

# The private key also contains the public key, so use that if a private key is provided.
ifeq ($(CONFIG_INTEL_BG_NEED_BPM_PRIV_KEY),y)
$(obj)/bpm_pub.pem: $(call strip_quotes, $(CONFIG_INTEL_BG_BPM_PRIV_KEY_FILE))
	openssl pkey -in $< -pubout > $@
else ifeq ($(CONFIG_INTEL_BG_NEED_BPM_PUB_KEY),y)
$(obj)/bpm_pub.pem: $(call strip_quotes, $(CONFIG_INTEL_BG_BPM_PUB_KEY_FILE))
	cp $< $@
endif

BG_PROV:=$(obj)/bg-prov
BG_CFG:=$(obj)/bg.json

ifneq ($(CONFIG_INTEL_BG_PROV_EXTERNAL_BIN),y)
$(BG_PROV):
	printf "    BG_PROV    building tool\n"
	cd 3rdparty/intel-sec-tools; \
	GO111MODULE=on go build -o $(top)/$@ cmd/bg-prov/*.go
else
$(BG_PROV): $(call strip_quotes, $(CONFIG_INTEL_BG_PROV_EXTERNAL_BIN_PATH))
	cp $< $@
endif

$(BG_CFG): $(call strip_quotes, $(CONFIG_INTEL_BG_BG_PROV_CFG_FILE))
	cp $(CONFIG_INTEL_BG_BG_PROV_CFG_FILE) $@

ifeq ($(CONFIG_INTEL_BG_GENERATE_BPM),y)
ifeq ($(CONFIG_INTEL_BG_BG_PROV_BPM_USE_CFG_FILE),y)
$(obj)/bpm_unsigned.bin: $(obj)/coreboot.rom $(BG_PROV) $(BG_CFG)
	printf "    BG_PROV    creating unsigned BPM using config file\n"
	$(BG_PROV) bpm-gen $@ $< --config=$(BG_CFG)
else
$(obj)/bpm_unsigned.bin: $(obj)/coreboot.rom $(BG_PROV)
	printf "    BG_PROV    creating unsigned BPM\n"
	$(BG_PROV) bpm-gen $@ $< --pmbpmversion=$(CONFIG_INTEL_BG_BPM_VERSION) \
				 --svn=$(CONFIG_INTEL_BG_BPM_SVN) \
				 --acmsvn=$(CONFIG_INTEL_BG_ACM_SVN) \
				 --nems=$$(($(CONFIG_DCACHE_RAM_SIZE)/0x1000)) \
				 --pbet=$(CONFIG_INTEL_BG_PBET) \
				 --ibbflags=$(CONFIG_INTEL_BG_IBB_FLAGS) \
				 --entrypoint=$(shell printf "%d" 0xfffffff0) \
				 --out=$(obj)/bpm_cfg.json
endif

ifeq ($(CONFIG_INTEL_BG_BPM_ONLY_UNSIGNED),y)
build_complete:: $(obj)/bpm_unsigned.bin
	@printf "\n** WARNING **\n"
	@printf "Build generated an unsigned BPM image: build/bpm_unsigned.bin.\n"
	@printf "The resulting image will not work with BG.\n"
	@printf "After you have externally signed the image you can add it to the coreboot image:\n"
	@printf "$$ cbfstool build/coreboot.rom add -f bpm.bin -n boot_policy_manifest.bin -t raw -a 16\n"
	@printf "$$ ifittool -r COREBOOT -a -n boot_policy_manifest.bin -t 12 -s 12 -f build/coreboot.rom\n"
else
$(obj)/bpm.bin: $(obj)/bpm_unsigned.bin $(BG_PROV) $(call strip_quotes, $(CONFIG_INTEL_BG_BPM_PRIV_KEY_FILE))
	printf "    BG_PROV    signing real BPM\n"
	$(BG_PROV) bpm-sign $< $@ $(CONFIG_INTEL_BG_BPM_PRIV_KEY_FILE) ""

# Add BPM at the end of the build when all files have been added
files_added:: $(obj)/bpm.bin
	printf "    BG       Adding BPM\n"
	$(CBFSTOOL) $(obj)/coreboot.rom add -f $< -n boot_policy_manifest.bin -b 0xffff1000 -t raw
	printf "    IFITTOOL   Adding BPM\n"
	$(IFITTOOL) -r COREBOOT -a -n boot_policy_manifest.bin -t 12 -s $(CONFIG_CPU_INTEL_NUM_FIT_ENTRIES) -f $(obj)/coreboot.rom

endif # CONFIG_INTEL_BG_BPM_ONLY_UNSIGNED

else # CONFIG_INTEL_BG_GENERATE_BPM

ifneq ($(CONFIG_INTEL_BG_BOOT_POLICY_MANIFEST_BINARY),"")
cbfs-files-y += boot_policy_manifest.bin
boot_policy_manifest.bin-file := $(CONFIG_INTEL_BG_BOOT_POLICY_MANIFEST_BINARY)
boot_policy_manifest.bin-type := raw
#boot_policy_manifest.bin-align := 0x10
boot_policy_manifest.bin-position := 0xffff1000

$(call add_intermediate, add_bpm_fit, $(IFITTOOL) set_fit_ptr)
	$(IFITTOOL) -r COREBOOT -a -n boot_policy_manifest.bin -t 12 -s $(CONFIG_CPU_INTEL_NUM_FIT_ENTRIES) -f $<
endif
endif # CONFIG_INTEL_BG_GENERATE_BPM

ifeq ($(CONFIG_INTEL_BG_GENERATE_KM),y)
ifeq ($(CONFIG_INTEL_BG_BG_PROV_KM_USE_CFG_FILE),y)
$(obj)/km_unsigned.bin: $(obj)/km_pub.pem $(BG_PROV) $(BG_CFG)
	printf "    BG_PROV    creating unsigned KM using config file\n"
	$(BG_PROV) km-gen $@ $< --config=$(BG_CFG)
else
$(obj)/km_unsigned.bin: $(obj)/km_pub.pem $(obj)/bpm_pub.pem $(BG_PROV)
	printf "    BG_PROV    creating unsigned KM\n"
	$(BG_PROV) km-gen $@ $< --kmversion=$(CONFIG_INTEL_BG_KM_VERSION) \
				--svn=$(CONFIG_INTEL_BG_KM_SVN) \
				--id=$(CONFIG_INTEL_BG_KM_ID) \
				--bpmpubkey=$(obj)/bpm_pub.pem \
				--bpmhashalgo=$(PK_HASH_ALG_SHA256) \
				--out=$(obj)/km_cfg.json
endif

$(obj)/km.bin: $(obj)/km_unsigned.bin $(BG_PROV) $(call strip_quotes, $(CONFIG_INTEL_BG_KM_PRIV_KEY_FILE))
	printf "    BG_PROV    signing KM\n"
	$(BG_PROV) km-sign $< $@ $(CONFIG_INTEL_BG_KM_PRIV_KEY_FILE) ""

KM_FILE=$(obj)/km.bin
else
KM_FILE=$(CONFIG_INTEL_BG_KEY_MANIFEST_BINARY)
endif

ifneq ($(KM_FILE),"")
ifeq ($(CONFIG_INTEL_BG_KM_ONLY_UNSIGNED),y)
$(call add_intermediate, gen_unsigned_km, $(obj)/km_unsigned.bin)
	@printf "Generating unsgined KM\n"

build_complete::
	@printf "\n** WARNING **\n"
	@printf "Build generated an unsigned KM image: build/km_unsiged.bin.\n"
	@printf "The resulting image will not work with BG.\n"
	@printf "After you have externally signed the image you can add it to the coreboot image:\n"
	@printf "$$ cbfstool build/coreboot.rom add -f km.bin -n key_manifest.bin -t raw -a 16\n"
	@printf "$$ ifittool -r COREBOOT -a -n key_manifest.bin -t 11 -s 12 -f build/coreboot.rom\n"

else
cbfs-files-y += key_manifest.bin
key_manifest.bin-file := $(KM_FILE)
key_manifest.bin-type := raw
#key_manifest.bin-align := 0x10
key_manifest.bin-position := 0xffff2000

$(call add_intermediate, add_km_fit, $(IFITTOOL) set_fit_ptr)
	$(IFITTOOL) -r COREBOOT -a -n key_manifest.bin -t 11 -s $(CONFIG_CPU_INTEL_NUM_FIT_ENTRIES) -f $<
endif

endif # CONFIG_INTEL_BG_KM_ONLY_UNSIGNED
endif # CONFIG_INTEL_BG_SUPPORT
