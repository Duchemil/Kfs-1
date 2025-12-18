#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 
bool caps_lock = false;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;



static inline void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void terminal_update_cursor(void)
{
	uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void)
{
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[(y-1) * VGA_WIDTH + x] =
                terminal_buffer[y * VGA_WIDTH + x];
        }
    } 

    // Clear last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
}


void terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(c, terminal_color);
        terminal_column++;
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }

    // Scroll si nécessaire
    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
        terminal_row = VGA_HEIGHT - 1;
    }

    // Met à jour le curseur physique
    terminal_update_cursor();
}

void delete_char()
{
	if (terminal_column == 0)
	{
		return;
	}
	else 
	{
		terminal_column --;
	}
	terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    terminal_update_cursor();
}


void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}


void terminal_writestring_color(const char* str, enum vga_color fg, enum vga_color bg)
{
	terminal_setcolor(vga_entry_color(fg, bg));
	terminal_writestring(str);
}

////////////////////////////////////////////////////////////////////////////
/*Ascii code for keyboard inputs*/
int	shift = 0;

char scancode_to_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0, '\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ',
    0,0,0,0,0,0,0,0,0,0,0, 0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
	
char scancode_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\'', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0 
};

unsigned char inb(unsigned short port)
{
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

uint8_t read_input()
{
    while(!(inb(0x64) & 1)){};
	return inb(0x60);
}
//////////////////////////////////////////////////////////////////////////////////////////

static void print_int(int value)
{
	if (value < 0) {
		terminal_putchar('-');
		value = -value;
	}

	char buf[12];
	int i = 0;

	if (value == 0) {
		terminal_putchar('0');
		return;
	}

	while (value > 0) {
		buf[i++] = '0' + (value % 10);
		value /= 10;
	}

	while (i--)
		terminal_putchar(buf[i]);
}

static void print_uint(unsigned int value)
{
	char buf[12];
	int i = 0;

	if (value == 0) {
		terminal_putchar('0');
		return;
	}

	while (value > 0) {
		buf[i++] = '0' + (value % 10);
		value /= 10;
	}

	while (i--)
		terminal_putchar(buf[i]);
}

static void print_hex(unsigned int value)
{
	char buf[9];
	int i = 0;

	if (value == 0) {
		terminal_putchar('0');
		return;
	}

	while (value > 0) {
		uint8_t digit = value & 0xF;
		buf[i++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
		value >>= 4;
	}

	terminal_writestring("0x");
	while (i--)
		terminal_putchar(buf[i]);
}

void printk(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	for (size_t i = 0; fmt[i]; i++) {
		if (fmt[i] != '%') {
			terminal_putchar(fmt[i]);
			continue;
		}

		i++; /* Skip '%' */

		switch (fmt[i]) {
			case 'c':
				terminal_putchar((char)va_arg(args, int));
				break;

			case 's': {
				const char *str = va_arg(args, const char*);
				terminal_writestring(str ? str : "(null)");
				break;
			}

			case 'd':
				print_int(va_arg(args, int));
				break;

			case 'u':
				print_uint(va_arg(args, unsigned int));
				break;

			case 'x':
				print_hex(va_arg(args, unsigned int));
				break;

			case '%':
				terminal_putchar('%');
				break;

			default:
				terminal_putchar('%');
				terminal_putchar(fmt[i]);
				break;
		}
	}

	va_end(args);
}



void keyboard_handler()
{
    uint8_t scancode = read_input();

	// printk("%x", scancode);
    if (scancode <= 0x3A)
    {
        if ((scancode == 0x2A) || scancode == 0x36 || caps_lock)
            shift = 1;
        else if (scancode == 0x0E)
            delete_char();
        else if (shift == 1)
            terminal_putchar(scancode_shift[scancode]);
        else
            terminal_putchar(scancode_to_ascii[scancode]);
    }
    // SHIFT
    else if(scancode <= 0x39 + 0x80)
    {
		if ((scancode - 0x80 == 0x2A) || (scancode - 0x80 == 0x36))
			shift = 0;
    }
}

void print_42_ascii(void)
{
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BROWN));
    terminal_writestring("____________/\\\\\\_______/\\\\\\\\\\\\\\\\\\_____        \n");
    terminal_writestring(" __________/\\\\\\\\\\_____/\\\\\\///////\\\\\\___       \n");
    terminal_writestring("  ________/\\\\\\/\\\\\\____\\///______\\//\\\\\\__      \n");
    terminal_writestring("   ______/\\\\\\/\\/\\\\\\______________/\\\\\\/___     \n");
    terminal_writestring("    ____/\\\\\\/__\\/\\\\\\___________/\\\\\\//_____    \n");
    terminal_writestring("     __/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_____/\\\\\\//________   \n");
    terminal_writestring("      _\\///////////\\\\\\//____/\\\\\\/___________  \n");
    terminal_writestring("       ___________\\/\\\\\\_____/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\_ \n");
    terminal_writestring("        Made by Llaigle and Lduchemi          \n");
}


void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();

	print_42_ascii();
	/* Newline support is left as an exercise. */
	terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
	terminal_writestring("It's yo boi CarlOS!\n");

	while(1){
		keyboard_handler();
	}

	// for(int i = 0; i < 999999; i++)
	// {
	// 	printk("String: %s\n", "Hello Kernel");
	// 	printk("Unsigned: %u\n", 123456);printk("CarlOS booting...\n");
	// 	printk("Answer = %d\n", 42);
	// 	printk("Hex: %x\n", 0x2A);
	// 	printk("Char: %c\n", 'A');
	// 	printk("String: %s\n", "Hello Kernel");
	// 	printk("Unsigned: %u\n", 123456);printk("CarlOS booting...\n");
	// 	printk("Answer = %d\n", 42);
	// 	printk("Hex: %x\n", 0x2A);
	// 	printk("Char: %c\n", 'A');
	// 	printk("String: %s\n", "Hello Kernel");
	// 	printk("Unsigned: %u\n", 123456);printk("CarlOS booting...\n");
	// 	printk("Answer = %d\n", 42);
	// 	printk("Hex: %x\n", 0x2A);
	// 	printk("Char: %c\n", 'A');
	// 	printk("String: %s\n", "Hello Kernel");
	// 	printk("Unsigned: %u\n", 123456);printk("CarlOS booting...\n");
	// 	printk("Answer = %d\n", 42);
	// 	printk("Hex: %x\n", 0x2A);
	// }
}