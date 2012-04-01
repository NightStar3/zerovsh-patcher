TARGET = zerovsh_upatcher
OBJS = main.o import.o 

INCDIR =
EXTRA_WARNS= -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wwrite-strings -Wunreachable-code
CFLAGS = -O2 -G0 -Wall -std=c99 $(EXTRA_WARNS) 

ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

PSP_FW_VERSION = 371

LDFLAGS = -mno-crt0 -nostartfiles
LIBS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
