#include "cga.h"

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

void cga_clear() {
	for(unsigned int iy = 0; iy < CGA_HEIGHT - 2; ++iy) {
		for(unsigned int ix = 0; ix < CGA_WIDTH; ++ix) {
			cga_screen[2*(iy*CGA_WIDTH + ix)] = ' ';
			cga_screen[2*(iy*CGA_WIDTH + ix) + 1] = cga_screen[2*((iy + 1)*CGA_WIDTH + ix) + 1];
		}
	}
}
