Test pour vérifier le multiboot : 
if grub-file --is-x86-multiboot carlos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi

Compiler kernel : i386-elf-gcc -c kernel.c -o kernel.o \
-ffreestanding -O2 -Wall -Wextra \
-fno-builtin -fno-stack-protector \
-std=gnu99

Compiler boot : i386-elf-as boot.s -o boot.o

Compiler linker : i386-elf-gcc -T linker.ld -o carlos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

Boot.s : Kernel entry points that setup the processor environment

Kernel.c : Actual kernel routines, whole "code" is here

Linker.ld : File to link both boot.s and kernel.c 