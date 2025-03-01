
obj-y += $(MDIR)exec.o
obj-y += $(MDIR)multiboot.o

dirs-y = fmt

include $(patsubst %,$(MDIR)%/module.mk,$(dirs-y))

