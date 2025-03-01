
obj-y += $(MDIR)bios.o
obj-y += $(MDIR)file.o

dirs-y = fs
dirs-$(CONFIG_PROTOCOL) += protocol

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

