# Directories
SRC_DIR := src
BUILD_DIR := build

# Toolchain
CC := gcc
CXX := g++
AS := nasm
COMMON_FLAGS := -Wall -Wextra -ffreestanding -O2 -m32 -fno-pie -fno-stack-protector \
		$(addprefix -I $(SRC_DIR)/, kernel gui apps fs drivers drivers/net drivers/input drivers/video)
CFLAGS := $(COMMON_FLAGS) -std=gnu99
CXXFLAGS := $(COMMON_FLAGS) -fno-exceptions -fno-rtti
LDFLAGS := -ffreestanding -O2 -nostdlib -m32 -Wl,-m,elf_i386 -no-pie -Wl,--build-id=none

# Files
BIN := $(BUILD_DIR)/lolos.bin
ISO := lolos.img

# Source files
SRCS := $(shell find $(SRC_DIR) -name "*.c" -or -name "*.cpp")
ASM_BIN_SRCS := $(shell find $(SRC_DIR)/bootloader -name "*.s")
ASM_ELF_SRCS := $(shell find $(SRC_DIR)/kernel -name "*.s")

# Object files
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
OBJS += $(ASM_ELF_SRCS:$(SRC_DIR)/kernel/%.s=$(BUILD_DIR)/%.o)

# Default target
all: $(ISO)

# Run/Emulate the OS
run: $(ISO)
	qemu-system-i386 -drive format=raw,file=$(ISO) -vga std -serial stdio -d int,cpu_reset -no-reboot

# Create bootable ISO
$(ISO): $(BIN) $(BUILD_DIR)/stage1.bin $(BUILD_DIR)/stage2.bin
	# Create a 1.44MB floppy image (or just enough for our needs)
	dd if=/dev/zero of=$(ISO) bs=512 count=2880
	# Write Stage 1 to Sector 1
	dd if=$(BUILD_DIR)/stage1.bin of=$(ISO) conv=notrunc
	# Write Stage 2 to Sector 2
	dd if=$(BUILD_DIR)/stage2.bin of=$(ISO) seek=1 conv=notrunc
	# Write Kernel to Sector 10
	objcopy -O binary $(BIN) $(BUILD_DIR)/kernel.bin
	dd if=$(BUILD_DIR)/kernel.bin of=$(ISO) seek=9 conv=notrunc

# Final binary
$(BIN): $(OBJS) $(SRC_DIR)/linker.ld
	$(CC) -T $(SRC_DIR)/linker.ld -o $@ $(LDFLAGS) $(OBJS)

# Assembly files
$(BUILD_DIR)/%.bin: $(SRC_DIR)/bootloader/%.s
	mkdir -p $(dir $@)
	$(AS) -f bin $< -o $@
$(BUILD_DIR)/%.o: $(SRC_DIR)/kernel/%.s
	mkdir -p $(dir $@)
	$(AS) -f elf32 $< -o $@

# C/C++ files
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Clean everything
clean:
	rm -rf $(BUILD_DIR) $(ISO)
