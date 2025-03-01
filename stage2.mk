# Stage 2 bootloader makefile

include .config

S2_BUILDDIR = $(BUILDDIR)/stage2

STAGE2      = $(S2_BUILDDIR)/stage2.bin

S2_SRCDIR = src
S2_INCDIR = inc

S2_LDFLAGS = -T stage2.ld -melf_i386
S2_CFLAGS  = -m32 -march=i386 -fno-pic \
			 -I $(S2_INCDIR) \
			 -nostdlib -nostdinc -ffreestanding -Wall -Wextra -Werror -Os \
			 -g -fno-stack-protector -fdata-sections -ffunction-sections
S2_ASFLAGS = $(S2_CFLAGS)

obj-y :=
cflags-y :=

include src/module.mk

S2_OBJS := $(filter %.o,$(patsubst %.o,$(S2_BUILDDIR)/%.o,$(obj-y)))
S2_DEPS := $(filter %.d,$(patsubst %.o,%.d,$(S2_OBJS)))

S2_CFLAGS += $(cflags-y)

$(STAGE2): $(S2_OBJS)
	@echo -e "\033[32m    \033[1mLD\033[21m    \033[34m$@\033[0m"
	$(Q) $(LD) $(S2_LDFLAGS) -o $(STAGE2).elf $(S2_OBJS)
	$(Q) $(OBJCOPY) -O binary --only-section=.text --only-section=.rodata --only-section=.data $(STAGE2).elf $@


$(S2_BUILDDIR)/%.o: %.c .config
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(S2_CFLAGS) -MMD -MP --save-temps -c -o $@ $<

$(S2_BUILDDIR)/%.o: %.s .config
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(S2_CFLAGS) -MMD -MP -c -o $@ $<

.config: | .defconfig
	@echo -e "\033[32m\033[1mCopying default .config\033[0m"
	$(Q) cp .defconfig $@


stage2_clean:
	$(Q) rm -f $(STAGE2) $(STAGE2).elf $(S2_OBJS) $(S2_DEPS)

.PHONY: stage2_clean

-include $(S2_DEPS)

