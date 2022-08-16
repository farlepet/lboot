# Stage 2 bootloader makefile

S2_BUILDDIR = $(BUILDDIR)/stage2

STAGE2      = $(S2_BUILDDIR)/stage2.bin

S2_SRCDIR = src
S2_INCDIR = inc

S2_LDFLAGS = -T stage2.ld -melf_i386
S2_CFLAGS  = -m32 -march=i386 -fno-pic \
			 -I $(S2_INCDIR) \
			 -nostdlib -nostdinc -ffreestanding -Wall -Wextra -Werror -O2 \
			 -g -fno-stack-protector -fdata-sections -ffunction-sections
S2_ASFLAGS = $(S2_CFLAGS)


S2_SRCS = $(S2_SRCDIR)/startup/startup.s \
		  $(S2_SRCDIR)/startup/cstart.c  \
		  $(S2_SRCDIR)/stdlib/string.c   \
		  $(S2_SRCDIR)/io/output.c       \
		  $(S2_SRCDIR)/io/vga.c          \
		  $(S2_SRCDIR)/bios/bios.s       \
		  $(S2_SRCDIR)/storage/bios.c    \
		  $(S2_SRCDIR)/storage/fs/fs.c   \
		  $(S2_SRCDIR)/storage/fs/fat.c

S2_OBJS = $(filter %.o,$(patsubst $(S2_SRCDIR)/%.c,$(S2_BUILDDIR)/%.o,$(S2_SRCS)) \
                       $(patsubst $(S2_SRCDIR)/%.s,$(S2_BUILDDIR)/%.o,$(S2_SRCS)))
S2_DEPS = $(filter %.d,$(patsubst $(S2_SRCDIR)/%.c,$(S2_BUILDDIR)/%.d,$(S2_SRCS)))

$(STAGE2): $(S2_OBJS)
	@echo -e "\033[32m    \033[1mLD\033[21m    \033[34m$@\033[0m"
	$(Q) $(LD) $(S2_LDFLAGS) -o $(STAGE2).elf $(S2_OBJS)
	$(Q) $(OBJCOPY) -O binary --only-section=.text --only-section=.rodata --only-section=.data $(STAGE2).elf $@

$(S2_BUILDDIR): $(BUILDDIR)
	$(Q) mkdir -p $@


$(S2_BUILDDIR)/%.o: $(S2_SRCDIR)/%.c
	@echo -e "\033[32m    \033[1mCC\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(S2_CFLAGS) -MMD -MP --save-temps -c -o $@ $<

$(S2_BUILDDIR)/%.o: $(S2_SRCDIR)/%.s
	@echo -e "\033[32m    \033[1mAS\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(S2_CFLAGS) -MMD -MP -c -o $@ $<




stage2_clean:
	$(Q) rm -f $(STAGE2) $(STAGE2).elf $(S2_OBJS) $(S2_DEPS)

.PHONY: stage2_clean

-include $(S2_DEPS)

