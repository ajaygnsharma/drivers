/*
 * include/asm-arm/arch-lpc22xx/board.h
 *
 *  Copyright (C) 2007 Siemens Building Technologies
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * These are data structures found in platform_device.dev.platform_data,
 * and describing board-specific data needed by drivers.  For example,
 * which pin is used for a given GPIO role.
 *
 * In 2.6, drivers should strongly avoid board-specific knowledge so
 * that supporting new boards normally won't require driver patches.
 * Most board-specific knowledge should be in arch/.../board-*.c files.
 */

#ifndef __ASM_ARCH_BOARD_H
#define __ASM_ARCH_BOARD_H



/* I2C */

struct lpc2xxx_i2c_pdata {
	int fpclk;
	int freq; /* I2C bus frequency */
	int timeout;
	int retries;
};

/* SPI */

struct lpc2xxx_spi_info {

	unsigned long		 board_size;
	struct spi_board_info	*board_info;

	void (*set_cs)(struct lpc2xxx_spi_info *spi, int cs, int pol);
};

/* SPI, SSD1289 Display controller */


struct ssd1289_platform_data {
	void (*control_mode)(int control);
};

/* LCD */

struct lpc2xxx_lcd_data {
	int fpclk;
};


#endif
