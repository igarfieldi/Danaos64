#include "cga.h"
#include "util/math.h"

static const unsigned int CGA_WIDTH = 80;
static const unsigned int CGA_HEIGHT = 25;
static const unsigned int CGA_ATTRIBUTE = 15;

static unsigned int x = 0;
static unsigned int y = 0;
static volatile char *cga_screen = (volatile char*) 0xB8000;

void scroll_down() {
	for(unsigned int iy = 0; iy < CGA_HEIGHT - 2; ++iy) {
		for(unsigned int ix = 0; ix < CGA_WIDTH; ++ix) {
			cga_screen[2*(iy*CGA_WIDTH + ix)] = cga_screen[2*((iy + 1)*CGA_WIDTH + ix)];
			cga_screen[2*(iy*CGA_WIDTH + ix) + 1] = cga_screen[2*((iy + 1)*CGA_WIDTH + ix) + 1];
		}
	}
	
	for(unsigned int ix = 0; ix < CGA_WIDTH; ++ix) {
		cga_screen[2*((CGA_HEIGHT - 1)*CGA_WIDTH + ix)] = ' ';
		cga_screen[2*((CGA_HEIGHT - 1)*CGA_WIDTH + ix) + 1] = CGA_ATTRIBUTE;
	}
}

void put_next(char c) {
	switch(c) {
		case '\t':
			x += 8;
			break;
		case '\n':
			x = 0;
			++y;
			break;
		default:
			cga_screen[2*(y*CGA_WIDTH + x)] = c;
			cga_screen[2*(y*CGA_WIDTH + x) + 1] = CGA_ATTRIBUTE;
			++x;
			break;
	}
	
	if(x >= CGA_WIDTH) {
		x = 0;
		++y;
	}
	if(y >= CGA_HEIGHT) {
		scroll_down();
		--y;
	}
}

void print(const char *msg) {
	while(*msg != '\0') {
		put_next(*msg);
		++msg;
	}
}

void print_int(int32_t val, unsigned char base) {
	if(val < 0) {
		put_next('-');
		val *= -1;
	}
	print_uint(val, base);
}

void print_uint(uint32_t val, unsigned char base) {
	uint32_t currFig, currDigit;

    switch (base) {
        case 2:
			put_next('0');
			put_next('b');
            break;
        case 16:
			put_next('0');
			put_next('x');
            break;
        default:
            break;
    }

    for (uint32_t i = logull(base, val) + 1; i > 0; i--) {
        currFig   = powull(base, i - 1);
        currDigit = val / currFig;

        // If currDigit > 9, continue with capital letters (only for hex base)
        put_next((char) (currDigit + 48 + (currDigit > 9 ? 7 : 0)));
        val -= currDigit * currFig;
    }
    
    if((base != 2) && (base != 10) && (base != 16)) {
    	put_next('_');
    	print_uint(base, 10);
    }
}

void cga_clear() {
	cga_screen = (volatile char*) 0xB8000;
	for(unsigned int iy = 0; iy < CGA_HEIGHT - 2; ++iy) {
		for(unsigned int ix = 0; ix < CGA_WIDTH; ++ix) {
			cga_screen[2*(iy*CGA_WIDTH + ix)] = ' ';
			cga_screen[2*(iy*CGA_WIDTH + ix) + 1] = cga_screen[2*((iy + 1)*CGA_WIDTH + ix) + 1];
		}
	}
	x = 0;
	y = 0;
}
