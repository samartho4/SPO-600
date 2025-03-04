.section .data
msg:     .asciz "Loop:  "   # Two spaces reserved for digits
newline: .asciz "\n"

.section .bss
buffer:  .space 2           # Reserve space for two characters

.section .text
.global _start

_start:
    mov     $0, %r15         # Counter starts at 0
    mov     $33, %r14        # Max value (loop condition: i < 33)

loop:
    # Print "Loop: "
    mov     $1, %rax
    mov     $1, %rdi
    lea     msg(%rip), %rsi
    mov     $7, %rdx         # Length of "Loop:  "
    syscall

    # Divide %r15 by 10
    mov     %r15, %rax
    xor     %rdx, %rdx       # Clear remainder register
    mov     $10, %rdi        # Divisor = 10
    div     %rdi             # %rax = quotient, %rdx = remainder

    # Convert quotient (tens digit) to ASCII and print
    add     $48, %al         # Convert to ASCII
    mov     %al, buffer      # Store character in buffer

    mov     $1, %rax
    mov     $1, %rdi
    lea     buffer(%rip), %rsi
    mov     $1, %rdx
    syscall

    # Convert remainder (ones digit) to ASCII and print
    mov     %dl, %al         # Move remainder to AL
    add     $48, %al         # Convert to ASCII
    mov     %al, buffer      # Store character in buffer

    mov     $1, %rax
    mov     $1, %rdi
    lea     buffer(%rip), %rsi
    mov     $1, %rdx
    syscall

    # Print newline
    mov     $1, %rax
    mov     $1, %rdi
    lea     newline(%rip), %rsi
    mov     $1, %rdx
    syscall

    # Increment counter
    inc     %r15
    cmp     %r15, %r14
    jne     loop

    # Exit syscall
    mov     $60, %rax
    xor     %rdi, %rdi
    syscall
