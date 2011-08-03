TARGET = zerovsh_patcher
OBJS = main.o import.o logger.o hook.o blacklist.o resolver.o
INCDIR =
CFLAGS = -O2 -G0 -Wall -std=c99 -Wshadow

all:
	-psp-packer zerovsh_patcher.prx zerovsh_patcher.prx

ifeq ($(DEBUG), 1)
CFLAGS+=-DDEBUG
endif

ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

PSP_FW_VERSION = 371

LDFLAGS = -mno-crt0 -nostartfiles
LIBS = 

USE_KERNEL_LIBS=1
USE_KERNEL_LIBC=1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
