# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = OS2024
HEADER_DIR = src/header
USER_HEADER_DIR = src/header/user
STDLIB_HEADER_DIR = src/header/stdlib

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS        = $(DEBUG_CFLAG) $(STRIP_CFLAG) -m32 -c -I $(SOURCE_FOLDER) -I ${HEADER_DIR} -I $(USER_HEADER_DIR) -I $(STDLIB_HEADER_DIR)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386

DISK_NAME = storage
SAMPLE_DISK_NAME = sample-image
COPY_SUFFIX = -copy

# @qemu-system-i386 -s -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso -no-reboot -d cpu_reset,int
all: copysampledisk insert-shell all
	@qemu-system-i386 -s -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso

run: 
	@qemu-system-i386 -s -drive file=$(OUTPUT_FOLDER)/$(DISK_NAME).bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

all: build
build: iso


KERNEL = kernel
SRC_KERNEL = $(KERNEL).c
OBJ_KERNEL = $(SRC_KERNEL:.c=.o)

SRC_ALL := $(wildcard $(SOURCE_FOLDER)/*/*.c)

SRC := $(filter-out $(SOURCE_FOLDER)/external/external-inserter.c, $(SRC_ALL))

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
	# Filter out the specific file
	$(eval FILTERED_OBJS := $(filter-out $(OUTPUT_FOLDER)/exclude_this_file.o,$(OBJS)))
	@$(LIN) $(LFLAGS) -o $(OUTPUT_FOLDER)/kernel $(OUTPUT_FOLDER)/$(OBJ_KERNEL) $(FILTERED_OBJS)
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

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g -I$(SOURCE_FOLDER) \
		$(SOURCE_FOLDER)/stdlib/string.c \
		$(SOURCE_FOLDER)/filesystem/fat32.c \
		$(SOURCE_FOLDER)/external/external-inserter.c \
		$(SOURCE_FOLDER)/cpu/portio.c \
		-o $(OUTPUT_FOLDER)/inserter





USER_MAIN = user-shell
USER_MAIN_SRC = $(USER_MAIN).c
USER_MAIN_OBJ = $(USER_MAIN_SRC:.c=.o)

USER_DIR = $(SOURCE_FOLDER)/user
STDLIB_DIR = $(SOURCE_FOLDER)/stdlib
USER_SRC_ALL = $(wildcard $(USER_DIR)/*.c) $(wildcard $(STDLIB_DIR)/*.c)
USER_OBJS = $(patsubst $(USER_DIR)/%,$(OUTPUT_FOLDER)/user/%,$(USER_SRC_ALL:.c=.o))
USER_OUTPUT_DIR = $(OUTPUT_FOLDER)/user

$(OUTPUT_FOLDER)/%.o: $(USER_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -fno-pie -c -o $@ $<
user-shell: CFLAGS += -fno-pie
user-shell: $(OUTPUT_FOLDER)/$(USER_MAIN_OBJ) $(USER_OBJS)
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/user/crt0.s -o crt0.o

	@$(LIN) -T $(SOURCE_FOLDER)/user/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o ${USER_OBJS} -o $(OUTPUT_FOLDER)/shell

	@echo Linking object shell object files and generate flat binary...

	@$(LIN) -T $(SOURCE_FOLDER)/user/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		crt0.o ${USER_OBJS} -o $(OUTPUT_FOLDER)/shell_elf
	@echo Linking object shell object files and generate ELF32 for debugging...

	@size --target=binary $(OUTPUT_FOLDER)/shell
	@rm -f *.o

insert-shell: inserter user-shell $(OUTPUT_FOLDER)/$(DISK_NAME).bin
	@echo Inserting shell into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter shell 2 $(DISK_NAME).bin
	@echo Inserting shell into root directory finished!




copydisk:
	@cp $(OUTPUT_FOLDER)/$(DISK_NAME).bin $(OUTPUT_FOLDER)/$(DISK_NAME).bin

copysampledisk:
	@cp $(OUTPUT_FOLDER)/$(SAMPLE_DISK_NAME).bin $(OUTPUT_FOLDER)/$(DISK_NAME).bin

clean:
	rm -rf $(wildcard $(OUTPUT_FOLDER)/*.iso) $(filter-out $(wildcard $(OUTPUT_FOLDER)/*.bin), $(wildcard $(OUTPUT_FOLDER)/*))
