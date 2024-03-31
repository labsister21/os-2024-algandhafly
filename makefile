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

run: all
	@qemu-system-i386 -s -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso -no-reboot -d cpu_reset,int

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

all: build
build: iso
clean:
	rm -rf $(OUTPUT_FOLDER)/*.o $(OUTPUT_FOLDER)/*.iso $(OUTPUT_FOLDER)/kernel $(OUTPUT_FOLDER)/cpu $(OUTPUT_FOLDER)/interrupt $(OUTPUT_FOLDER)/stdlib $(OUTPUT_FOLDER)/text $(OUTPUT_FOLDER)/driver



kernel:
	@echo Linking object files and generate elf32...
	@mkdir -p $(OUTPUT_FOLDER)/cpu $(OUTPUT_FOLDER)/interrupt $(OUTPUT_FOLDER)/stdlib $(OUTPUT_FOLDER)/text $(OUTPUT_FOLDER)/driver
	
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/kernel-entrypoint.s -o $(OUTPUT_FOLDER)/kernel-entrypoint.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/interrupt/intsetup.s -o $(OUTPUT_FOLDER)/interrupt/intsetup.o

	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/kernel.c -o $(OUTPUT_FOLDER)/kernel.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/cpu/gdt.c -o $(OUTPUT_FOLDER)/cpu/gdt.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/cpu/portio.c -o $(OUTPUT_FOLDER)/cpu/portio.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/text/framebuffer.c -o $(OUTPUT_FOLDER)/text/framebuffer.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/text/stringdrawer.c -o $(OUTPUT_FOLDER)/text/stringdrawer.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/interrupt.c -o $(OUTPUT_FOLDER)/interrupt/interrupt.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/interrupt/idt.c -o $(OUTPUT_FOLDER)/interrupt/idt.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/driver/keyboard.c -o $(OUTPUT_FOLDER)/driver/keyboard.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/driver/disk.c -o $(OUTPUT_FOLDER)/driver/disk.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/filesystem/fat32.c -o $(OUTPUT_FOLDER)/filesystem/fat32.o
	$(CC) $(CFLAGS) $(SOURCE_FOLDER)/stdlib/string.c -o $(OUTPUT_FOLDER)/stdlib/string.o
	@$(LIN) $(LFLAGS) bin/*.o bin/cpu/*.o bin/interrupt/*.o bin/text/*.o bin/driver/*.o bin/filesystem/*.o bin/stdlib/*.o -o $(OUTPUT_FOLDER)/kernel
	@rm -f *.o
	@echo Linking object files and generate elf32 finished!

iso: kernel
	@echo Generating iso file...
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
	@genisoimage -R -b boot/grub/grub1 -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o $(OUTPUT_FOLDER)/OS2024.iso $(OUTPUT_FOLDER)/iso
	@rm -r $(OUTPUT_FOLDER)/iso/
	@echo New iso file generated!
