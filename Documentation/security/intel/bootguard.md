# Intel BootGuard

Intel Boot Guard introduces a hardware-based trust anchor.

It consists of five different parts:

1.  Hash of Key Manifest(KM) public signing key fused into Field Programming Fuse(FPF).
    (Trust-Anchor/Root-of-Trust) in the chipset.
2.  KM signed with the KM private signing key. KM contains the hash of Boot Policy
    Manifest(BPM) public signing key.
3.  BPM signed with BPM private signing key. BPM contains platform specific configuration
    information. (IBB information, hashes of IBB, etc.)
4.  BootGuard Authenticated Code Module [ACM]
5.  Firmware Interface Table[FIT] entries of KM, BPM and ACM.

Intel BootGuard requirements:

1.  Intel Flash Image Tool (requires NDA with Intel Corp.)
2.  Intel BootGuard requires signed **Authenticated Code Module** ([ACM]s), provided
    by Intel. (requires NDA with Intel Corp.)
3.  Intel BootGuard requres **CPU and Chipset** support (supported since Intel Haswell U)

## Authenticated Code Modules

The ACMs are Intel digitally signed modules that contain code to be run
before the traditional x86 CPU reset vector.

ACMs for Intel BootGuard are not distributed publicly, so a signed NDA
with Intel Corp. is required.

More details can be found here: [ACM]

## For developers
### Configuring Intel BootGuard in Kconfig
Enable ``INTEL_BG_SUPPORT`` for general usage.

### Generate KM and BPM
Enable ``INTEL_BG_GENERATE_BPM`` and ``INTEL_BG_GENERATE_KM`` if a new Key Manifest and
Bootpolicy Manifest should be generated.

If the options stay disabled you need to provide paths for ``INTEL_BG_KEY_MANIFEST_BINARY``
and ``INTEL_BG_BOOT_POLICY_MANIFEST_BINARY``.



#### Generate KM and BPM with json config
Set ``INTEL_BG_BG_PROV_CFG_FILE`` with path of the json configuration file. For further information about
how to generate a configuration json file and its content see: [Intel-Sec-Tools](../../../3rdparty/intel-sec-tools/cmd/bg/README.md)

#### Generate KM and BPM with args
Key Manifest options:
- INTEL_BG_KM_VERSION
- INTEL_BG_KM_SVN
- INTEL_BG_KM_ID

Bootpolicy Manifest options:
- INTEL_BG_BPM_VERSION
- INTEL_BG_BPM_SVN
- INTEL_BG_ACM_SVN
- INTEL_BG_PBET
- INTEL_BG_IBB_FLAGS

For further information on these options see Intel document #557867.

### Key paths for KM and BPM private key
Set ``INTEL_BG_KM_PRIV_KEY_FILE`` and ``INTEL_BG_BPM_PRIV_KEY_FILE`` paths.


[FIT]: ../../soc/intel/fit.md
[ACM]: acm.md
