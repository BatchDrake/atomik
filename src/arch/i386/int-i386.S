/*
 *    int.S: Low level interrupt handling
 *    Copyright (C) 2016  Gonzalo J. Carracedo
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


.global i386_idt_flush
.global i386_idle_task
.global return_to_kernel

.text
.align   4

.macro ISR_ERRCODE num
    .global isr\num
    .align   4
 isr\num:

    pushl $\num
    jmp __intr_common
.endm

.macro ISR_NOERRCODE num
    .global isr\num
    .align   4
  isr\num:
        
    pushl $0
    pushl $\num
    jmp __intr_common
.endm

.macro ISR_IRQ num
    .global isr\num
    .align   4
  isr\num:
    pushl $0
    pushl $\num
    jmp __intr_common
.endm

i386_idle_task:
    hlt
    jmp i386_idle_task

i386_idt_flush:
    movl 4(%esp), %eax
    lidt (%eax)
    ret
 
__intr_common:   
    .extern curr_tcb    /* Current TCB */
    .extern i386_handle_kernel_interrupt
    .extern i386_handle_user_interrupt
    cld                /* Clear direction flag */
    pushl %eax
    mov %cs, %ax
    test $3, %ax       /* Do we come from userland? */
    jnz save_tcb
    popl %eax          /* Nope, this is a kernel fault */
    pusha
    pushl %esp
    call i386_handle_kernel_interrupt
    addl $4, %esp
    popa

return_to_kernel:
    jmp  isr_done

save_tcb:               /* Interrupt was raised in userland */
    movl curr_tcb, %eax
        
    movl %ebx,  4(%eax)
    movl %ecx,  8(%eax)
    movl %edx, 12(%eax)
    movl %esi, 16(%eax)
    movl %edi, 20(%eax)
    movl %ebp, 24(%eax)

    /* Stack configuration upon privilege change
       is like this:

       esp + 28  userss
       esp + 24  useresp
       esp + 20  eflags
       esp + 16  cs
       esp + 12  eip
       esp + 8   error
       esp + 4   intno
       esp + 0   saved eax

      We should ignore current esp and save useresp
      instead. */

    movl (%esp), %ebx   /* eax */
    movl %ebx, (%eax)
        
    movl 24(%esp), %ebx /* Useresp */
    movl %ebx, 28(%eax)

    movl 20(%esp), %ebx /* eflags */
    movl %ebx, 32(%eax)

    movl 12(%esp), %ebx /* eip */
    movl %ebx, 36(%eax)

    call i386_handle_user_interrupt

return_to_user:
    /* Restore context */
    movl curr_tcb, %eax

    movl 28(%eax), %ebx
    movl %ebx, 24(%esp) /* Useresp */
    
    movl 32(%eax), %ebx
    movl %ebx, 20(%esp) /* eflags */

    movl 36(%eax), %ebx
    movl %ebx, 12(%esp) /* eip */

    movl 4(%eax),  %ebx
    movl 8(%eax),  %ecx
    movl 12(%eax), %edx
    movl 16(%eax), %esi
    movl 20(%eax), %eax
    movl 24(%eax), %ebp

    movl (%eax),   %eax /* This must be the last one */
        
isr_done:
    addl $8, %esp
    iret

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_NOERRCODE 12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

/* IRQ interrupts */

ISR_IRQ 32
ISR_IRQ 33
ISR_IRQ 34
ISR_IRQ 35
ISR_IRQ 36
ISR_IRQ 37
ISR_IRQ 38
ISR_IRQ 39
ISR_IRQ 40
ISR_IRQ 41
ISR_IRQ 42
ISR_IRQ 43
ISR_IRQ 44
ISR_IRQ 45
ISR_IRQ 46
ISR_IRQ 47

/* IOAPIC interrupts */

ISR_IRQ 48
ISR_IRQ 49
ISR_IRQ 50
ISR_IRQ 51
ISR_IRQ 52
ISR_IRQ 53
ISR_IRQ 54
ISR_IRQ 55

/* User-defined interrupt gates */
ISR_NOERRCODE 56
ISR_NOERRCODE 57
ISR_NOERRCODE 58
ISR_NOERRCODE 59
ISR_NOERRCODE 60
ISR_NOERRCODE 61
ISR_NOERRCODE 62
ISR_NOERRCODE 63
ISR_NOERRCODE 64
ISR_NOERRCODE 65
ISR_NOERRCODE 66
ISR_NOERRCODE 67
ISR_NOERRCODE 68
ISR_NOERRCODE 69
ISR_NOERRCODE 70
ISR_NOERRCODE 71
ISR_NOERRCODE 72
ISR_NOERRCODE 73
ISR_NOERRCODE 74
ISR_NOERRCODE 75
ISR_NOERRCODE 76
ISR_NOERRCODE 77
ISR_NOERRCODE 78
ISR_NOERRCODE 79
ISR_NOERRCODE 80
ISR_NOERRCODE 81
ISR_NOERRCODE 82
ISR_NOERRCODE 83
ISR_NOERRCODE 84
ISR_NOERRCODE 85
ISR_NOERRCODE 86
ISR_NOERRCODE 87
ISR_NOERRCODE 88
ISR_NOERRCODE 89
ISR_NOERRCODE 90
ISR_NOERRCODE 91
ISR_NOERRCODE 92
ISR_NOERRCODE 93
ISR_NOERRCODE 94
ISR_NOERRCODE 95
ISR_NOERRCODE 96
ISR_NOERRCODE 97
ISR_NOERRCODE 98
ISR_NOERRCODE 99
ISR_NOERRCODE 100
ISR_NOERRCODE 101
ISR_NOERRCODE 102
ISR_NOERRCODE 103
ISR_NOERRCODE 104
ISR_NOERRCODE 105
ISR_NOERRCODE 106
ISR_NOERRCODE 107
ISR_NOERRCODE 108
ISR_NOERRCODE 109
ISR_NOERRCODE 110
ISR_NOERRCODE 111
ISR_NOERRCODE 112
ISR_NOERRCODE 113
ISR_NOERRCODE 114
ISR_NOERRCODE 115
ISR_NOERRCODE 116
ISR_NOERRCODE 117
ISR_NOERRCODE 118
ISR_NOERRCODE 119
ISR_NOERRCODE 120
ISR_NOERRCODE 121
ISR_NOERRCODE 122
ISR_NOERRCODE 123
ISR_NOERRCODE 124
ISR_NOERRCODE 125
ISR_NOERRCODE 126
ISR_NOERRCODE 127
ISR_NOERRCODE 128
ISR_NOERRCODE 129
ISR_NOERRCODE 130
ISR_NOERRCODE 131
ISR_NOERRCODE 132
ISR_NOERRCODE 133
ISR_NOERRCODE 134
ISR_NOERRCODE 135
ISR_NOERRCODE 136
ISR_NOERRCODE 137
ISR_NOERRCODE 138
ISR_NOERRCODE 139
ISR_NOERRCODE 140
ISR_NOERRCODE 141
ISR_NOERRCODE 142
ISR_NOERRCODE 143
ISR_NOERRCODE 144
ISR_NOERRCODE 145
ISR_NOERRCODE 146
ISR_NOERRCODE 147
ISR_NOERRCODE 148
ISR_NOERRCODE 149
ISR_NOERRCODE 150
ISR_NOERRCODE 151
ISR_NOERRCODE 152
ISR_NOERRCODE 153
ISR_NOERRCODE 154
ISR_NOERRCODE 155
ISR_NOERRCODE 156
ISR_NOERRCODE 157
ISR_NOERRCODE 158
ISR_NOERRCODE 159

/* Microkernel system call */
ISR_NOERRCODE 160

/* Bugcheck interrupt */
ISR_NOERRCODE 255
