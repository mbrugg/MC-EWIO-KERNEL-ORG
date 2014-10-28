/*/*
 * include/asm-arm./arch-at91/at91_ks8x93_device.h
 *
 * (C) Copyright 2006
 * Forschungs- und Transferzentrum (FTZ) Leipzig
 * Mirco Fuchs <fuchs@ftz-leipzig.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef AT91_KS8x93_DEVICE_H_
#define AT91_KS8x93_DEVICE_H_

 /* KS8x93 device data */
struct at91_ks8x93_data {
	u8	reset_pin;	/* pin for external reset */
	u8	cfg_sel_pin0;	/* PS0: SPI or SMI mode (if PS1 is 1) */
	u8	is_smi;		/* using SMI interface? */
	u8	is_rmii;	/* using RMII interface? */
};
extern void __init at91_add_device_ks8x93(struct at91_ks8x93_data *data);

#endif /* AT91_KS8x93_DEVICE_H_ */
