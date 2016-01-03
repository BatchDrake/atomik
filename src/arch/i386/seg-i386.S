/*
 *    seg-i386.S: Low level segment management for i386
 *    Copyright (C) 2016 Gonzalo J. Carracedo
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

.globl i386_get_current_gdt
.globl i386_flush_gdt
.globl i386_refresh_segments
.globl i386_flush_tss
.globl i386_enter_user

i386_flush_tss:
    movw $0x2b, %ax
    ltr %ax
    ret

i386_get_current_gdt:
    movl 4(%esp), %eax
    sgdt (%eax)
    ret

i386_flush_gdt:
    movl 4(%esp), %eax
    lgdt (%eax)
    
    call __refresh_segments
    
    ret

__refresh_segments:
  jmpl $0x8, $.reload
  
.reload:
  pushl %eax
  movw $0x10, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss
  popl %eax
  ret
  
i386_enter_user:
  movw $0x23, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movl %esp, %eax
  pushl $0x23
  pushl %eax
  pushf
  pushl $0x1b
  pushl $.user_entry
  iret
        
.user_entry:
  ret
        
  
