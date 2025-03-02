# Stage 2

MDIR = $(dir $(lastword $(MAKEFILE_LIST)))

dirs-y := bios    \
          config  \
          data    \
          exec    \
          intr    \
          io      \
          mm      \
          startup \
          stdlib  \
          storage \
          time

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

cflags-$(CONFIG_VERBOSE_PANIC) += -DCONFIG_VERBOSE_PANIC
cflags-$(CONFIG_EXCEPTIONS)    += -DCONFIG_VERBOSE_EXCEPTIONS
cflags-$(CONFIG_STATUSBAR)     += -DCONFIG_STATUSBAR
cflags-$(CONFIG_WORKINGSTATUS) += -DCONFIG_WORKINGSTATUS

# Debug config
cflags-y += -DDEBUG_CONFIG=$(CONFIG_DEBUG_CONFIG) \
            -DDEBUG_STORAGE_BIOS=$(CONFIG_DEBUG_STORAGE_BIOS) \
            -DDEBUG_FS_FAT=$(CONFIG_DEBUG_FS_FAT) \
            -DDEBUG_EXEC=$(CONFIG_DEBUG_EXEC) \
            -DDEBUG_EXEC_ELF=$(CONFIG_DEBUG_EXEC_ELF) \
            -DDEBUG_EXEC_MULTIBOOT=$(CONFIG_DEBUG_EXEC_MULTIBOOT) \
            -DDEBUG_XMODEM=$(CONFIG_DEBUG_PROTOCOL_XMODEM) \

