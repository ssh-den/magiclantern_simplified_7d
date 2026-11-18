MODEL = 7D
FW_VERSION = 206

TOP_DIR = ../..

AUTOEXEC_BASE = 0x800000
FIR_BASE = 0x800120

FIRMWARE_ID = 0x80000250

# The 7D is dual DIGIC 4. ML runs on the "slave" CPU, whose ROM is at FF010000.
MAIN_FIRMWARE_ADDR = 0xFF010000
PLATFORM_ARCH = armv5te

# ML is placed at the top of the AllocateMemory heap; boot-d45-ch.c moves
# the heap end down to RESTARTSTART (see consts.h and heap-bounds.lds).
RESTARTSTART = 0xC60000
ML_BOOT_OBJ = boot-d45-ch.o

ML_SRC_EXTRA_OBJS = \
    $(BUILD_DIR)/ml_rpc.o \
    $(BUILD_DIR)/video_hacks.o \
    $(BUILD_DIR)/ptpbuf.o \
    $(BUILD_DIR)/afma.o \
    $(BUILD_DIR)/full-runtime.o

include ../Makefile

CFLAGS += -DCONFIG_7D206_FULL_CORE
$(BUILD_DIR)/full-runtime.o: CFLAGS += -Wformat=2 -Werror=format

# reboot.c uses .incbin; it must embed this build's payload/version files.
INCLUDE_ARGS = -I. -Iinclude -I$(BUILD_DIR) -I$(SRC_DIR)

# Resolve the remaining module libc imports from the bundled archive.
$(BUILD_DIR)/magiclantern: $(DIETLIBC_A)
$(BUILD_DIR)/magiclantern: heap-bounds.lds

# RESTARTSTART is compiled into objects and the generated linker script.
$(ML_OBJS) $(ML_LD_SCRIPT): Makefile

.DEFAULT_GOAL := $(BUILD_DIR)/cam_complete
