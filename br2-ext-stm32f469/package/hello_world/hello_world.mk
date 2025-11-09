################################################################################
# Hello world - simple example package
################################################################################

HELLO_WORLD_VERSION = 1.0
HELLO_WORLD_SITE = "$(BR2_EXTERNAL_SS_STM32F469_EXT_PATH)/package/hello_world/src"
HELLO_WORLD_SITE_METHOD = local

HELLO_WORLD_LICENSE = Proprietary
HELLO_WORLD_LICENSE_FILES =

# If you had deps, list them here (e.g. HELLO_WORLD_DEPENDENCIES = zlib)
HELLO_WORLD_DEPENDENCIES =

define HELLO_WORLD_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define HELLO_WORLD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hello_world \
		$(TARGET_DIR)/usr/bin/hello_world
endef

$(eval $(generic-package))
