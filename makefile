# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = OS2024

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS        = $(DEBUG_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386

DISK_NAME = storage

# @qemu-system-i386 -s -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso -no-reboot -d cpu_reset,int
run: all
	@qemu-system-i386 -s -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

all: build
build: iso


KERNEL = kernel
SRC_KERNEL = $(KERNEL).c
OBJ_KERNEL = $(SRC_KERNEL:.c=.o)
SRC = $(wildcard $(SOURCE_FOLDER)/*/*.c)
SRC_ASM = $(wildcard $(SOURCE_FOLDER)/*.s) $(wildcard $(SOURCE_FOLDER)/*/*.s)
OBJS = $(patsubst $(SOURCE_FOLDER)/%,$(OUTPUT_FOLDER)/%,$(SRC:.c=.o)) $(patsubst $(SOURCE_FOLDER)/%,$(OUTPUT_FOLDER)/%,$(SRC_ASM:.s=.o))

$(OUTPUT_FOLDER)/%.o: $(SOURCE_FOLDER)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT_FOLDER)/%.o: $(SOURCE_FOLDER)/%.s
	mkdir -p $(@D)
	@$(ASM) $(AFLAGS) -o $@ $<

$(OUTPUT_FOLDER)/%.o: %.s
	mkdir -p $(@D)
	@$(ASM) $(AFLAGS) -o $@ $<

kernel: $(OUTPUT_FOLDER)/$(OBJ_KERNEL) $(OBJS)
	@echo Linking object files...
	@$(LIN) $(LFLAGS) -o $(OUTPUT_FOLDER)/kernel $^
	@rm -f *.o
	@echo Linking object files finished!


iso: kernel
	@echo Generating iso file...
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
	@echo Generating elf32...
	@genisoimage -R -b boot/grub/grub1 -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o $(OUTPUT_FOLDER)/OS2024.iso $(OUTPUT_FOLDER)/iso
	@echo Generating elf32 finished!
	@rm -r $(OUTPUT_FOLDER)/iso/
	@echo New iso file generated!


clean:
	rm -rf $(wildcard $(OUTPUT_FOLDER)/*.iso) $(filter-out $(wildcard $(OUTPUT_FOLDER)/*.bin), $(wildcard $(OUTPUT_FOLDER)/*))
