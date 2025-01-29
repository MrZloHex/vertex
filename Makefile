MCU        := atmega328p
F_CPU      := 16000000
PROGRAMMER := arduino

CC         := avr-gcc
OBJCOPY    := avr-objcopy
SIZE       := avr-size
AVRDUDE    := avrdude

CFLAGS      = -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS     += -Wall -Wextra -Wpedantic -std=c2x -Wstrict-aliasing
CFLAGS     += -Iinc -Ilib
LDFLAGS     = -Wl,--gc-sections -mmcu=$(MCU)

TARGET      = vertex

SRC         = src
OBJ         = obj
BIN         = bin
LIB         =
TST         =

SOURCES     = $(wildcard $(SRC)/*.c)
LIBRARY     = $(wildcard $(LIB)/*.c)
OBJECTS     = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))
OBJECTS    += $(patsubst $(LIB)/%.c, $(OBJ)/%.o, $(LIBRARY))

ELF         = $(BIN)/$(TARGET).elf
HEX         = $(BIN)/$(TARGET).hex



all: dirs $(HEX)

dirs:
	-mkdir -v obj bin

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(ELF): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ)/%.o: $(LIB)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

size: $(ELF)
	$(SIZE) --format=berkeley $(ELF) 

clean:
	-rm $(BIN)/*
	-rm $(OBJECTS)

flash: $(HEX)
	$(AVRDUDE) -p $(MCU) -P $(PORT) -c $(PROGRAMMER) -e -U flash:w:$(HEX)

.PHONY: all flash clean