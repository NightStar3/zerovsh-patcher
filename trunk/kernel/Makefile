TARGET = zerovsh_patcher
OBJS = main.o import.o blacklist.o resolver.o hook.o minini/minIni.o
INCDIR =
EXTRA_WARNS= -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wwrite-strings -Wunreachable-code
MININI_DEFINES = -DNDEBUG -DINI_READONLY -DINI_FILETYPE=SceUID -DPORTABLE_STRNICMP -DINI_NOFLOAT
CFLAGS = -O2 -G0 -Wall -std=c99 $(EXTRA_WARNS) $(MININI_DEFINES)

ifeq ($(DEBUG), 1)
CFLAGS+=-DDEBUG
endif

ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

PSP_FW_VERSION = 371

LDFLAGS = -mno-crt0 -nostartfiles

USE_KERNEL_LIBS=1
USE_KERNEL_LIBC=1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
