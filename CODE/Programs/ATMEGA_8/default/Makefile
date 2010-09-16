###############################################################################
# Makefile for the project tut5lite
###############################################################################

## General Flags
PROJECT = tut5lite
MCU = atmega8
TARGET = atmega_8.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -O0 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=atmega_8.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"C:\Pumpkin\Salvo\Example\ATMEGA_8\..\..\Inc" -I"C:\Pumpkin\Salvo\Example\ATMEGA_8\." 

## Library Directories
LIBDIRS = -L"C:\Pumpkin\Salvo\Lib\GCCAVR" 

## Libraries
LIBS = -lsalvofgccavr-t 

## Objects that must be built in order to link
OBJECTS = atmega8.o salvomem.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) atmega_8.hex atmega_8.eep atmega_8.lss size

## Compile
atmega8.o: ../atmega8.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

salvomem.o: ../../../Src/salvomem.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) atmega_8.elf dep/* atmega_8.hex atmega_8.eep atmega_8.lss atmega_8.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

