MAINDIR    = .
BUILDDIR   = $(MAINDIR)/build/$(ARCH)/$(CPU)/$(HW)

SRC        = $(MAINDIR)/src
INC        = $(MAINDIR)/inc

ifeq ($(VERBOSE), 1)
Q =
else
Q = @
endif

SRCS       = $(wildcard $(SRC)/*.c)
OBJS       = $(filter %.o,$(patsubst $(SRC)/%.c,$(BUILDDIR)/%.o,$(SRCS)))
DEPS       = $(filter %.d,$(patsubst $(SRC)/%.c,$(BUILDDIR)/%.d,$(SRCS)))

CFLAGS    += -Wall -Wextra -Werror -I$(INC) -O2

ifeq ($(CC), clang)
CFLAGS    += -Weverything -Wno-padded
endif

OUT        = sector_mapper

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJS)
	@echo -e "\033[33m  \033[1mLinking sources\033[0m"
	@$(CC) -o $(OUT) $(OBJS)

$(BUILDDIR)/%.o: $(SRC)/%.c
	@echo -e "\033[32m  \033[1mCC\033[21m    \033[34m$<\033[0m"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(CFLAGS) -MMD -MP -c -o $@ $<


clean:
	@rm -f $(OBJS) $(OUT)


-include $(DEPS)
