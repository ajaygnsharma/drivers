[![STM32F469I Discovery — Zephyr Project Documentation](https://tse4.mm.bing.net/th/id/OIP.oACEE36pwsvmyQ_ZUV9R8AAAAA?cb=ucfimg2\&pid=Api\&ucfimg=1)](https://docs.zephyrproject.org/latest/boards/st/stm32f469i_disco/doc/index.html?utm_source=chatgpt.com)

Nice board choice 😄

I’ll keep your **existing external tree skeleton** and just adapt the bits that need to be STM32F469-Discovery-specific:

* custom DTS that matches `stm32f469i-disco`
* use Buildroot’s **built-in stm32f469 defconfig** as the base
* hook your **myapp** sample package into it

I’ll assume your external tree from before is now called:

```text
br2-ext-stm32f469/
├── external.desc
├── external.mk
├── Config.in
├── configs/
│   └── stm32f469_myext_sd_defconfig   (we’ll generate this)
├── board/
│   └── st/
│       └── ss-stm32f469-disco/
│           └── ss-stm32f469.dts
└── package/
    └── hello_world/
        ├── Config.in
        ├── hello_world.mk
        └── src/
            ├── Makefile
            └── main.c
```

---

## 1. External tree boilerplate (tiny tweaks)

**`external.desc`**

```text
name: SS_STM32F469_EXT
desc: External tree for STM32F469-Discovery (custom DTS + sample app)
```

**`external.mk`**

(same idea as before, just path changes)

```make
include $(sort $(wildcard $(BR2_EXTERNAL_SS_STM32F469_EXT_PATH)/package/*/*.mk))
```

> Note: Buildroot turns `name: STM32F469_EXT` into `BR2_EXTERNAL_STM32F469_EXT_PATH`. ([Buildroot][1])

**`Config.in`**

```kconfig
menu "External options (STM32F469_EXT)"

source "$BR2_EXTERNAL_SS_STM32F469_EXT_PATH/package/hello_world/Config.in"

endmenu
```

---

## 2. Board-specific custom DTS

Create:

`br2-ext-stm32f469/board/st/ss-stm32f469-disco/ss-stm32f469.dts`

Minimal, Linux-bootable skeleton that matches the real board name and SoC. Recent kernels have a `stm32f469.dtsi` SoC file you can include. ([FOSDEM Archive][2])

```dts
/dts-v1/;

#include "stm32f469.dtsi"

/ {
    model = "STMicroelectronics STM32F469I-Discovery (custom)";
    compatible = "st,stm32f469i-disco", "st,stm32f469";

    /* On this board SRAM is aliased at 0x00000000 in the Linux config */
    memory {
        device_type = "memory";
        reg = <0x00000000 0x01000000>; /* 16 MiB mapped for Linux; adjust if needed */
    };

    chosen {
        /* U-Boot for this board typically uses USART3 as the console */
        bootargs = "console=ttyS2,115200 root=/dev/ram";
        stdout-path = "serial0:115200n8";
    };

    aliases {
        serial0 = &usart3;
    };

    /* Simple vbus regulator example - optional */
    vcc5v_otg: vcc5v-otg-regulator {
        compatible = "regulator-fixed";
        regulator-name = "vcc5v_otg";
        enable-active-high;
        gpio = <&gpiob 2 0>;
        regulator-always-on;
    };
};

/* Typical clock + peripheral enable bits, tweak as you go */

&clk_hse {
    clock-frequency = <8000000>;
    status = "okay";
};

&rcc {
    status = "okay";
};

&usart3 {
    status = "okay";
};
```

This is intentionally **small**; you’ll gradually add LTDC/DSI, QSPI, SDMMC, etc. The official DTS for this board is much longer, but you don’t need all of it just to have a clean “custom DTS folder” example. ([Android Source][3])

---

## 3. Re-use Buildroot’s stm32f469-disco defconfig

Buildroot already ships good configs for this board:

* `stm32f469_disco_sd_defconfig` – kernel + rootfs on SD
* `stm32f469_disco_xip_defconfig` – tiny XIP image in on-chip flash ([Buildroot Lists][4])

You don’t need to reinvent those; just:

### Step 1 – Start from the official SD defconfig

From your Buildroot tree:

```bash
cd /path/to/buildroot

# Use the built-in stm32f469 SD-card configuration
make stm32f469_disco_sd_defconfig
```

This already sets:

* ARM Cortex-M4 **no-MMU** toolchain
* Linux 5.15.x with the patches Buildroot ships for this board
* U-Boot `stm32f469-discovery` board config
* proper genimage + post-build scripts for this board ([Buildroot Lists][5])

### Step 2 – Enable your external tree and custom DTS

Now bring in your external tree and tweak config:

```bash
make BR2_EXTERNAL=/path/to/br2-ext-stm32f469 menuconfig
```

In `menuconfig`:

1. **Hook up your custom DTS**

   * `Kernel` → `Kernel binary format / Device Tree and firmware`
   * Enable:

     * `[*] Build a Device Tree Blob (DTB)`
     * `[*] Using a custom device tree source`
   * Set:

     ```text
     ($BR2_EXTERNAL_SS_STM32F469_EXT_PATH)/board/st/stm32f469-disco/my-stm32f469.dts
     BR2_LINUX_KERNEL_CUSTOM_DTS_PATH
     ```

   That tells Buildroot to copy your DTS from the external tree into the kernel tree and compile it instead of (or in addition to) the one from the board patches. ([Buildroot Lists][6])

2. **Enable your sample app**

   * `External options (STM32F469_EXT)` → `myapp (sample app)`
   * Select `[*] myapp` (from `package/myapp/Config.in`).

### Step 3 – Save this as an external defconfig (optional but nice)

If you’d like a **named defconfig inside your external tree**, do:

```bash
# Still in the Buildroot top dir
make savedefconfig BR2_DEFCONFIG=$BR2_EXTERNAL_STM32F469_EXT_PATH/configs/stm32f469_myext_sd_defconfig
```

From now on you can configure with a single command:

```bash
make BR2_EXTERNAL=/path/to/br2-ext-stm32f469 stm32f469_myext_sd_defconfig
```

---

## 4. Build and run

Normal Buildroot flow:

```bash
# Configure (if you didn’t save your external defconfig yet)
make BR2_EXTERNAL=../br2-ext-stm32f469/ stm32f469_disco_sd_defconfig
make BR2_EXTERNAL=../br2-ext-stm32f469/ menuconfig   # set DTS + myapp once
make

# Or, if you *did* save stm32f469_myext_sd_defconfig
make BR2_EXTERNAL=../br2-ext-stm32f469/ stm32f469_myext_sd_defconfig
make

# Make application
make BR2_EXTERNAL=../br2-ext-stm32f469/ hello_world-dirclean
make BR2_EXTERNAL=../br2-ext-stm32f469/ hello_world-rebuild
make
```

Results:

* **Kernel + DTB** and SD-card images in `output/images/` (from the official board support). ([GitLab][7])
* `/usr/bin/hello_world` in the target rootfs (same hello-world app as before).

---

If you want, I can next:

* turn this into a tiny README for `br2-ext-stm32f469`, or
* show how to base your custom DTS off the *full* upstream `stm32f469-disco.dts` (graphics, SD, QSPI, etc.) and then slowly strip/add things for your use case.

[1]: https://buildroot.org/downloads/manual/manual.html?utm_source=chatgpt.com "The Buildroot user manual"
[2]: https://archive.fosdem.org/2024/events/attachments/fosdem-2024-2864-linux-can-upstreaming-on-mmu-less-systems/slides/22435/Linux_CAN_upstreaming_on_MMU_less_systems_N5WZk7u.pdf?utm_source=chatgpt.com "Linux CAN upstreaming on MMU-less systems"
[3]: https://android.googlesource.com/kernel/common/%2B/cuttlefish-4.14/arch/arm/boot/dts/stm32f469-disco.dts?utm_source=chatgpt.com "arch/arm/boot/dts/stm32f469-disco.dts - kernel/common"
[4]: https://lists.buildroot.org/pipermail/buildroot/2025-January/771193.html?utm_source=chatgpt.com "[Buildroot] [PATCH 1/1] configs/stm32f469_disco_{sd, xip}"
[5]: https://lists.buildroot.org/pipermail/buildroot/2016-March/521077.html?utm_source=chatgpt.com "new configuration for STM32F469 Discovery board"
[6]: https://lists.buildroot.org/pipermail/buildroot/2018-March/580621.html?utm_source=chatgpt.com "[Buildroot] [PATCH 1/2] package/linux: fix custom dts files ..."
[7]: https://gitlab.savoirfairelinux.com/netdsa/buildroot/-/tree/11271540bfe6adafbc133caf6b5b902a816f5f02/board/stmicroelectronics/stm32f469-disco?utm_source=chatgpt.com "stm32f469-disco - netdsa / buildroot"
