# ===== Toolchain =====
TARGET  := i386-elf
CC      := $(TARGET)-gcc
AS      := $(TARGET)-as
LD      := $(TARGET)-ld

# ===== Directories =====
SRC_DIR     := src
BUILD_DIR   := build
ISO_DIR     := iso

# ===== Files =====
KERNEL_BIN  := carlos.bin
ISO_IMAGE   := carlos.iso

ASM_SRC     := $(SRC_DIR)/boot.s
C_SRC       := $(SRC_DIR)/kernel.c
LINKER      := $(SRC_DIR)/linker.ld

ASM_OBJ     := $(BUILD_DIR)/boot.o
C_OBJ       := $(BUILD_DIR)/kernel.o

# ===== Flags =====
CFLAGS  := -ffreestanding -O2 -Wall -Wextra -fno-builtin -fno-stack-protector -std=gnu99 
LDFLAGS := -T $(LINKER)

# ===== Default =====
all: $(ISO_IMAGE)

# ===== Directories =====
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(ISO_DIR):
	mkdir -p $(ISO_DIR)/boot/grub

# ===== Build objects =====
$(ASM_OBJ): $(ASM_SRC) | $(BUILD_DIR)
	$(AS) $< -o $@

$(C_OBJ): $(C_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ===== Link kernel =====
$(BUILD_DIR)/$(KERNEL_BIN): $(ASM_OBJ) $(C_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# ===== Create ISO =====
$(ISO_IMAGE): $(BUILD_DIR)/$(KERNEL_BIN) | $(ISO_DIR)
	cp $(BUILD_DIR)/$(KERNEL_BIN) $(ISO_DIR)/boot/$(KERNEL_BIN)
	cp boot/grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR)

# ===== Run =====
run: $(ISO_IMAGE)
	qemu-system-i386 -cdrom $(ISO_IMAGE)

# ===== Debug =====
check: $(BUILD_DIR)/$(KERNEL_BIN)
	grub-file --is-x86-multiboot $(BUILD_DIR)/$(KERNEL_BIN)

# ===== Cleanup =====
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO_IMAGE)

.PHONY: all run clean check
