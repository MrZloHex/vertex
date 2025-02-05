# ==============================================================================
#
#	 ███╗   ███╗ ██████╗ ███╗   ██╗ ██████╗ ██╗     ██╗████████╗██╗  ██╗
#	 ████╗ ████║██╔═══██╗████╗  ██║██╔═══██╗██║     ██║╚══██╔══╝██║  ██║
#	 ██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║██║     ██║   ██║   ███████║
#	 ██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║██║     ██║   ██║   ██╔══██║
#	 ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║╚██████╔╝███████╗██║   ██║   ██║  ██║
#	 ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚══════╝╚═╝   ╚═╝   ╚═╝  ╚═╝
#
#                           ░▒▓█ _VERTEX_█▓▒░
#
#   Makefile
#   Author     : MrZloHex
#   Date       : 2025-02-05
#   Version    : 1.0
#
#   Description:
#       This Makefile compiles and links the tma project sources.
#       It searches recursively under the "src" directory for source files,
#       compiles them into "obj", and links the final executable in "bin".
#
#   Warning    : This Makefile is so cool it might make your terminal shine!
# ==============================================================================
#
# Verbosity: Set V=1 for verbose output (full commands) or leave it unset for cool, quiet messages.

V ?= 0
ifeq ($(V),0)
	Q = @
else
	Q =
endif

BUILD 		?= debug


CC			 = avr-gcc
CP			 = avr-objcopy
SZ			 = avr-size
AD			 = avrdude

CFLAGS		 = -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS		+= -Wall -Wextra -Wpedantic -std=c2x -Wstrict-aliasing
CFLAGS		+= -MMD -MP
CFLAGS		+= -Iinc -Ilib

ifeq ($(BUILD),debug)
	CFLAGS	+= -Og -g
else ifeq ($(BUILD),release)
	CFLAGS	+= -O2 -Werror
else
	$(error Unknown build mode: $(BUILD). Use BUILD=debug or BUILD=release)
endif

LDFLAGS		 = -Wl,--gc-sections -mmcu=$(MCU)


TARGET		 = vertex

MCU			 = atmega328p
F_CPU		 = 16000000
PROGRAMMER	 = arduino

SRC			 = src
OBJ			 = obj
BIN			 = bin
LIB			 =
TST			 =


SOURCES		 = $(shell find $(SRC) -type f -name '*.c')
OBJECTS		 = $(addprefix $(OBJ)/, $(SOURCES))

ifneq ($(strip $(LIB)),)
LIBRARY		 = $(wildcard $(LIB)/*.c)
OBJECTS		+= $(addprefix $(OBJ)/, $(LIBRARY))
endif

OBJECTS		:= $(patsubst %.c, %.o, $(OBJECTS))


all: $(BIN)/$(TARGET).elf $(BIN)/$(TARGET).hex

$(BIN)/$(TARGET).elf: $(OBJECTS)
	@mkdir -p $(BIN)
	@echo "  CCLD     $(patsubst $(BIN)/%,%,$@)"
	$(Q) $(CC) -o $@ $^ $(LDFLAGS)

$(OBJ)/%.o: %.c
	@mkdir -p $(@D)
	@echo "  CC       $(patsubst $(OBJ)/%,%,$@)"
	$(Q) $(CC) -o $@ -c $< $(CFLAGS) -Wa,-a,-ad,-alms=$(OBJ)/$(notdir $(<:.c=.lst))

ifneq ($(strip $(LIB)),)
$(OBJ)/%.o: $(LIB)/%.c
	@mkdir -p $(@D)
	@echo "  CC       $(patsubst $(OBJ)/%,%,$@)"
	$(Q) $(CC) -o $@ -c $< $(CFLAGS)
endif

$(BIN)/%.hex: $(BIN)/%.elf
	@echo "  CP 	   $(patsubst $(BIN)/%,%,$@)"
	$(Q) $(CP) -O ihex -R .eeprom $< $@

clean:
	$(Q) rm -rf $(OBJ) $(BIN)

debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

flash: $(BIN)/$(TARGET).hex
	@echo " UPLOADING $(BIN)/$(TARGET).hex"
	$(Q) $(AD) -p $(MCU) -P $(PORT) -c $(PROGRAMMER) -e -U flash:w:$(BIN)/$(TARGET).hex

.PHONY: all clean debug release

-include $(OBJECTS:.o=.d)
