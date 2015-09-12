/*
 *    i386-serial.h: The x86 serial port.
 *    Copyright (C) 2014  Gonzalo J. Carracedo
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef _ARCH_I386_SERIAL_H
#define _ARCH_I386_SERIAL_H

#define COM_PORT_1_IO 0x3f8
#define COM_PORT_2_IO 0x2f8
#define COM_PORT_3_IO 0x3e8
#define COM_PORT_4_IO 0x2e8

int  i386_serial_putchar (uint16_t, char);
void i386_serial_init (void);
  
#endif /* _ARCH_I386_SERIAL_H */
