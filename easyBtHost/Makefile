
PROJECT		= easyBtHost.exe
#TOOL_CHAIN_PREFIX      = arm-none-eabi-
PROJECT_DIR  = .
OPTIMIZE     = 0
CONFIG_FLAGS = -g -O0
LDLIBS		 = -lusb-1.0 -pthread
#LDLIBS		 = -lusb -Lusb
#CONFIG_FLAGS += -m32
-include AutoConfig/.config
#include source.mk
SOURCES		= \
			  $(shell find "." -maxdepth 1 -name "*.c") \
			  $(shell find "usb" -maxdepth 1 -name "*.c") \

INCLUDES	= \
			  -I$(PROJECT_DIR)\
			  -Iusb\

############################################################################################

# build obj files direcotry
BUILDDIR	= .build

TRACE_CC  = @echo "	CC    $<"
TRACE_AS  = @echo "	AS    $<"
TRACE_LD  = @echo "Linking ..."

# Target
TARGET					:= $(PROJECT)

# Toolchain
CC			= $(TOOL_CHAIN_PREFIX)gcc
AS			= $(TOOL_CHAIN_PREFIX)gcc -x assembler-with-cpp
LD			= $(TOOL_CHAIN_PREFIX)ld
AR			= $(TOOL_CHAIN_PREFIX)ar
OBJCP		= $(TOOL_CHAIN_PREFIX)objcopy
OBJDUMP		= $(TOOL_CHAIN_PREFIX)objdump
OBJSIZE		= $(TOOL_CHAIN_PREFIX)size

# Optimize
ifeq ($(OPTIMIZE), ultrasize)
CC_OPTIMIZE = -Os -fdata-sections -ffunction-sections
LD_OPTIMIZE = -Wl,--gc-sections
else
CC_OPTIMIZE = -O$(OPTIMIZE)
LD_OPTIMIZE =
endif

# Warnings
WARNINGS 	= -Wall -Wformat=0 -Wstrict-aliasing=0

# Flags
CCFLAGS		= -std=gnu99  $(CONFIG_FLAGS) $(WARNINGS) $(CC_OPTIMIZE) $(INCLUDES)
ASFLAGS		= $(CPU) $(CONFIG_FLAGS) $(INCLUDES)
LDFLAGS		= $(CCFLAGS) $(LD_OPTIMIZE) #-Xlinker -Map=$(TARGET_MAP)
DEPFLAGS	= -MT $@ -MMD -MP -MF $*.d

# Files
FILES_C_OBJ = $(patsubst %,$(BUILDDIR)/%,$(filter %.o, $(SOURCES:%.c=%.o)))
FILES_S_OBJ = $(patsubst %,$(BUILDDIR)/%,$(filter %.o, $(SOURCES:%.S=%.o)))
FILES_DEPEND = $(patsubst %,$(BUILDDIR)/%,$(filter %.d, $(SOURCES:%.c=%.d)))


# PHONY
.PHONY: all clean print document lib

# Target
all:  $(FILES_C_OBJ) $(FILES_S_OBJ)
	$(TRACE_LD)
	@$(CC) $+ $(LDFLAGS) $(LDLIBS) -o $(TARGET)

# c -> o
$(BUILDDIR)/%.o : %.c
$(BUILDDIR)/%.o : %.c $(BUILDDIR)/%.d
	$(TRACE_CC)
	@mkdir -p $(@D) >/dev/null
	@$(CC) -MT $@ -MMD -MP -MF $(BUILDDIR)/$*.Td $(CCFLAGS) -c $< -o $@
	@mv -f $(BUILDDIR)/$*.Td $(BUILDDIR)/$*.d

# d
$(BUILDDIR)/%.d: ;
.PRECIOUS: $(BUILDDIR)/%.d

# Include dependent (*.d)
-include $(FILES_DEPEND)

print:
	@for f in $(SOURCES); do echo $$f; done
	@echo link script: $(LDSCRIPT)
	@for f in $(INCLUDES); do echo $$f; done

clean:
	@rm -rf $(BUILDDIR) $(TARGET)

