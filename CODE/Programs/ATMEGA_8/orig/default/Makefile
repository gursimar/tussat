###############################################################################
# Makefile for the project tut5lite
###############################################################################

## General Flags
PROJECT = tut5lite
MCU = atmega16
TARGET = tut5lite.elf
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
LDFLAGS +=  -Wl,-Map=tut5lite.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"C:\Pumpkin\Salvo\Example\AVR\ATmega16\AVR_Studio\Tut\Tut5\WinAVR\Lite\..\..\..\..\..\..\..\..\Inc" -I"C:\Pumpkin\Salvo\Example\AVR\ATmega16\AVR_Studio\Tut\Tut5\WinAVR\Lite\." 

## Library Directories
LIBDIRS = -L"C:\Pumpkin\Salvo\Lib\GCCAVR" 

## Libraries
LIBS = -lsalvofgccavr-t 

## Objects that must be built in order to link
OBJECTS = salvomem.o tut5.o tut5_board.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) tut5lite.hex tut5lite.eep tut5lite.lss size

## Compile
salvomem.o: ../../../../../../../../../Src/salvomem.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

tut5.o: ../../../../../../../AT90S8515/AVR_Studio/Tut/Tut5/tut5.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

tut5_board.o: ../../tut5_board.c
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
	-rm -rf $(OBJECTS) tut5lite.elf dep/* tut5lite.hex tut5lite.eep tut5lite.lss tut5lite.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

