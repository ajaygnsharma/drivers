################################################################################
#
# kmod-hello
#
################################################################################

KMOD_HELLO_VERSION = 1.0
KMOD_HELLO_SITE = $(BR2_EXTERNAL_SS_STM32F469_EXT_PATH)/package/kmod_hello
KMOD_HELLO_SITE_METHOD = local
KMOD_HELLO_LICENSE = GPL-2.0
KMOD_HELLO_DEPENDENCIES = linux

# Build the module against the kernel Buildroot is building
define KMOD_HELLO_BUILD_CMDS
	$(MAKE) -C $(LINUX_DIR) M=$(@D) \
		ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(TARGET_CROSS)" \
		modules
endef

# Install into the target rootfs under /lib/modules/$(uname -r)/
define KMOD_HELLO_INSTALL_TARGET_CMDS
	$(MAKE) -C $(LINUX_DIR) M=$(@D) \
		INSTALL_MOD_PATH=$(TARGET_DIR) \
		modules_install
endef

# (Optional) also install headers/sources into staging if you like
# define KMOD_HELLO_INSTALL_STAGING_CMDS
# endef

$(eval $(generic-package))
